#include <cairo.h>
#include <complex.h>
#include <gtk/gtk.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <omp.h>

#include "overlays.h"
#include "pixel.h"
#include "state.h"
#include "window.h"

#define SIZE 800
#define OVERLAYS_COLOR                                                         \
  { 0.82, 0.63, 0.00 }

#define INITIAL_JULIA false
#define INITIAL_JULIA_Z0 0 + 0.5 * I

#define INITIAL_MAX_ITER 128
#define INITIAL_COMPLEX_WIDTH 3
#define INITIAL_SCREEN_CENTER_AS_COMPLEX 0

State state;

static int8_t diverging_threshold(double complex initial_z, double complex c,
                                  int max_iter) {
  double complex z = initial_z;
  for (int i = 0; i < max_iter; i++) {
    z = z * z + c;
    if (creal(z) * creal(z) + cimag(z) * cimag(z) > 4) {
      return i;
    }
  }
  return -1;
}

void set_pixel_color(guchar *pixels, int x, int y, int r, int g, int b) {
  pixels[(y * SIZE + x) * 3] = r;
  pixels[(y * SIZE + x) * 3 + 1] = g;
  pixels[(y * SIZE + x) * 3 + 2] = b;
}

void color_point(Pixel *pixel) {
  double complex complex_point = pixel_get_complex_plane_coordinates(pixel);
  double complex screen_point = pixel_get_screen_coordinates(pixel);

  int8_t threshold;

  if (state.julia) {
    threshold = diverging_threshold(
        complex_point, pixel_get_complex_plane_coordinates(&state.julia_z0),
        state.max_iter);
  } else {
    threshold = diverging_threshold(0, complex_point, state.max_iter);
  }

  double color;
  if (threshold == -1) {
    color = 0;
  } else {
    color = threshold * 255.0 / 40;
    if (color > 255)
      color = 255;
  }

  set_pixel_color(state.pixels, creal(screen_point), cimag(screen_point), color,
                  color, color);

  int mirrored_y =
      2 * cimag(pixel_get_complex_plane_coordinates(&state.screen_center)) -
      cimag(screen_point);

  if (!state.julia && mirrored_y < state.window->size && mirrored_y >= 0) {
    set_pixel_color(state.pixels, creal(screen_point), mirrored_y, color, color,
                    color);
  }
}

static void draw(GtkDrawingArea *drawing_area, cairo_t *cr, int width,
                 int height, gpointer user_data) {

  double complex screen_center_in_complex_plane =
      pixel_get_complex_plane_coordinates(&state.screen_center);

  if (!state.julia) {
    if (cimag(screen_center_in_complex_plane) > height / 2.0) {
#pragma omp parallel for
      for (int x = 0; x < width; x++) {
        for (int y = 0; y < cimag(screen_center_in_complex_plane); y++) {
          Pixel pixel = pixel_new_from_screen_coordinates(&state, x + y * I);
          color_point(&pixel);
        }
      }
    } else {
#pragma omp parallel for
      for (int x = 0; x < width; x++) {
        for (int y = cimag(screen_center_in_complex_plane); y < height; y++) {
          Pixel pixel = pixel_new_from_screen_coordinates(&state, x + y * I);
          color_point(&pixel);
        }
      }
    }
  } else {
#pragma omp parallel for
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
        Pixel pixel = pixel_new_from_screen_coordinates(&state, x + y * I);
        color_point(&pixel);
      }
    }
  }

  GdkPixbuf *pixbuf =
      gdk_pixbuf_new_from_data(state.pixels, GDK_COLORSPACE_RGB, FALSE, 8,
                               width, height, width * 3, NULL, NULL);

  gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
  cairo_paint(cr);
  g_object_unref(pixbuf);

  if (state.show_overlays)
    draw_overlays(cr, &state);
}

static gboolean on_key_press(GtkEventControllerKey *controller, guint keyval,
                             guint keycode, GdkModifierType modifier) {

  GtkDrawingArea *drawing_area = state.window->drawing_area;

  switch (keyval) {
  case GDK_KEY_Up:
    state.screen_center =
        pixel_add_value(&state.screen_center, 0.1 * state.complex_width * I,
                        COORDINATES_TYPE_COMPLEX_PLANE);
    state.julia_z0._screen_coordinates_cached = false;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_Down:
    state.screen_center =
        pixel_add_value(&state.screen_center, -0.1 * state.complex_width * I,
                        COORDINATES_TYPE_COMPLEX_PLANE);
    state.julia_z0._screen_coordinates_cached = false;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_Right:
    state.screen_center =
        pixel_add_value(&state.screen_center, 0.1 * state.complex_width,
                        COORDINATES_TYPE_COMPLEX_PLANE);
    state.julia_z0._screen_coordinates_cached = false;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_Left:
    state.screen_center =
        pixel_add_value(&state.screen_center, -0.1 * state.complex_width,
                        COORDINATES_TYPE_COMPLEX_PLANE);
    state.julia_z0._screen_coordinates_cached = false;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_plus:
  case GDK_KEY_equal:
    state.complex_width *= 0.9;
    state.max_iter += 3;
    state.julia_z0._screen_coordinates_cached = false;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_minus:
    state.complex_width *= 1.1;
    state.max_iter -= 3;
    state.julia_z0._screen_coordinates_cached = false;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_q:
    gtk_window_destroy(GTK_WINDOW(state.window->app_window));
    break;
  case GDK_KEY_j:
    state.julia = !state.julia;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_h:
    state.screen_center = pixel_new_from_complex_plane_coordinates(&state, 0);
    state.complex_width = 3;
    state.max_iter = INITIAL_MAX_ITER;
    state.julia_z0 =
        pixel_new_from_complex_plane_coordinates(&state, INITIAL_JULIA_Z0);
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_w:
    state.julia_z0 = pixel_add_value(&state.julia_z0, 0.1 * I,
                                     COORDINATES_TYPE_COMPLEX_PLANE);
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_s:
    state.julia_z0 = pixel_add_value(&state.julia_z0, -0.1 * I,
                                     COORDINATES_TYPE_COMPLEX_PLANE);
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_a:
    state.julia_z0 =
        pixel_add_value(&state.julia_z0, -0.1, COORDINATES_TYPE_COMPLEX_PLANE);
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_d:
    state.julia_z0 =
        pixel_add_value(&state.julia_z0, +0.1, COORDINATES_TYPE_COMPLEX_PLANE);
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_o:
    state.show_overlays = !state.show_overlays;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  }

  return TRUE;
}

Pixel initial_julia_z0;

void on_drag_start(GtkGestureDrag *gesture, gdouble start_x, gdouble start_y,
                   gpointer user_data) {
  initial_julia_z0 = state.julia_z0;
}

void on_drag_update(GtkGestureDrag *gesture, gdouble offset_x, gdouble offset_y,
                    gpointer user_data) {

  if (!state.julia)
    return;

  state.julia_z0 = pixel_add_value(&initial_julia_z0, offset_x + offset_y * I,
                                   COORDINATES_TYPE_SCREEN);

  gtk_widget_queue_draw(GTK_WIDGET(state.window->drawing_area));
}

int main(int argc, char *argv[]) {
  state.pixels = malloc(SIZE * SIZE * 3);

  state.window = window_new("Fractales", SIZE, state.pixels, draw, on_key_press,
                            on_drag_start, on_drag_update);

  state.julia = INITIAL_JULIA;
  state.julia_z0 =
      pixel_new_from_complex_plane_coordinates(&state, INITIAL_JULIA_Z0);
  state.max_iter = INITIAL_MAX_ITER;
  state.complex_width = INITIAL_COMPLEX_WIDTH;
  state.screen_center = pixel_new_from_complex_plane_coordinates(
      &state, INITIAL_SCREEN_CENTER_AS_COMPLEX);

  state.show_overlays = true;
  const float overlays_color[3] = OVERLAYS_COLOR;
  state.overlays_color[0] = overlays_color[0];
  state.overlays_color[1] = overlays_color[1];
  state.overlays_color[2] = overlays_color[2];

  state.tick_step = 1;

  int status = window_present(state.window);

  window_free(state.window);

  g_free(state.pixels);

  return status;
}
