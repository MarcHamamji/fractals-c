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

#define INITIAL_MAX_ITER 256
#define INITIAL_COMPLEX_WIDTH 3
#define INITIAL_SCREEN_CENTER_AS_COMPLEX 0

#define INITIAL_NEWTON_ROOTS 3
#define INITIAL_NEWTON_ITERATIONS 20

State state;

static double complex newton_f(double complex *roots, unsigned int num_roots,
                               double complex x) {
  double complex result = 1;
  for (int i = 0; i < num_roots; i++) {
    result *= (x - roots[i]);
  }
  return result;
}

static double complex newton_f_prime(double complex *roots,
                                     unsigned int num_roots, double complex x) {
  double step = 1e-5;
  return (newton_f(roots, num_roots, x + step) -
          newton_f(roots, num_roots, x - step)) /
         (2 * step);
}

static int newton_threshold(double complex z0, State *state) {
  double complex z = z0;

  for (int i = 0; i < state->fractals_config.newton.iterations; i++) {
    z -= newton_f(state->fractals_config.newton.roots,
                  state->fractals_config.newton.num_roots, z) /
         newton_f_prime(state->fractals_config.newton.roots,
                        state->fractals_config.newton.num_roots, z);
  }

  int closest_root_index = 0;
  double closest_distance = cabs(z - state->fractals_config.newton.roots[0]);
  for (int root_index = 1;
       root_index < sizeof(state->fractals_config.newton.roots); root_index++) {
    double distance = cabs(z - state->fractals_config.newton.roots[root_index]);
    if (distance < closest_distance) {
      closest_distance = distance;
      closest_root_index = root_index;
    }
  }

  return closest_root_index;
}

static int8_t diverging_threshold(double complex initial_z, double complex c,
                                  int max_iter) {
  if (state.fractal_type == FRACTAL_MANDELBROT) {
    double p =
        csqrt((creal(c) - 0.25) * (creal(c) - 0.25) + cimag(c) * cimag(c));
    if (creal(c) < p - 2 * p * p + 0.25) {
      return -1;
    }
  }

  double complex z = initial_z;
  for (int i = 0; i < max_iter; i++) {
    z = z * z + c;
    if (creal(z) * creal(z) + cimag(z) * cimag(z) > 4) {
      return i;
    }
  }
  return -1;
}

static void set_pixel_color_rgb(guchar *pixels, int x, int y, int r, int g,
                                int b) {
  pixels[(y * SIZE + x) * 3] = r;
  pixels[(y * SIZE + x) * 3 + 1] = g;
  pixels[(y * SIZE + x) * 3 + 2] = b;
}

void color_point(Pixel *pixel) {
  double complex complex_point = pixel_get_complex_plane_coordinates(pixel);
  double complex screen_point = pixel_get_screen_coordinates(pixel);

  int8_t threshold;

  if (state.fractal_type == FRACTAL_JULIA) {
    threshold = diverging_threshold(
        complex_point,
        pixel_get_complex_plane_coordinates(&state.fractals_config.julia.z0),
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

  set_pixel_color_rgb(state.pixels, creal(screen_point), cimag(screen_point),
                      color, color, color);

  int mirrored_y =
      2 * cimag(pixel_get_complex_plane_coordinates(&state.screen_center)) -
      cimag(screen_point);

  if (state.fractal_type == FRACTAL_MANDELBROT &&
      mirrored_y < state.window->size && mirrored_y >= 0) {
    set_pixel_color_rgb(state.pixels, creal(screen_point), mirrored_y, color,
                        color, color);
  }
}

static void draw(GtkDrawingArea *drawing_area, cairo_t *cr, int width,
                 int height, gpointer user_data) {

  double complex screen_center_in_complex_plane =
      pixel_get_complex_plane_coordinates(&state.screen_center);

  if (state.fractal_type == FRACTAL_MANDELBROT) {
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

  } else if (state.fractal_type == FRACTAL_JULIA) {
#pragma omp parallel for
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
        Pixel pixel = pixel_new_from_screen_coordinates(&state, x + y * I);
        color_point(&pixel);
      }
    }
  } else if (state.fractal_type == FRACTAL_NEWTON) {
#pragma omp parallel for
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
        Pixel pixel = pixel_new_from_screen_coordinates(&state, x + y * I);
        int closest_root_index = newton_threshold(
            pixel_get_complex_plane_coordinates(&pixel), &state);
        double complex screen_point = pixel_get_screen_coordinates(&pixel);

        set_pixel_color_rgb(
            state.pixels, creal(screen_point), cimag(screen_point),
            closest_root_index * 255.0 / state.fractals_config.newton.num_roots,
            closest_root_index * 255.0 / state.fractals_config.newton.num_roots,
            closest_root_index * 255.0 /
                state.fractals_config.newton.num_roots);
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
    state.fractals_config.julia.z0._screen_coordinates_cached = false;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_Down:
    state.screen_center =
        pixel_add_value(&state.screen_center, -0.1 * state.complex_width * I,
                        COORDINATES_TYPE_COMPLEX_PLANE);
    state.fractals_config.julia.z0._screen_coordinates_cached = false;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_Right:
    state.screen_center =
        pixel_add_value(&state.screen_center, 0.1 * state.complex_width,
                        COORDINATES_TYPE_COMPLEX_PLANE);
    state.fractals_config.julia.z0._screen_coordinates_cached = false;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_Left:
    state.screen_center =
        pixel_add_value(&state.screen_center, -0.1 * state.complex_width,
                        COORDINATES_TYPE_COMPLEX_PLANE);
    state.fractals_config.julia.z0._screen_coordinates_cached = false;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_plus:
  case GDK_KEY_equal:
    state.complex_width *= 0.9;
    state.max_iter += 3;
    state.fractals_config.julia.z0._screen_coordinates_cached = false;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_minus:
    state.complex_width *= 1.1;
    state.max_iter -= 3;
    state.fractals_config.julia.z0._screen_coordinates_cached = false;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_q:
    gtk_window_destroy(GTK_WINDOW(state.window->app_window));
    break;
  case GDK_KEY_j:
    if (state.fractal_type == FRACTAL_MANDELBROT) {
      state.fractal_type = FRACTAL_JULIA;
    } else if (state.fractal_type == FRACTAL_JULIA) {
      state.fractal_type = FRACTAL_NEWTON;
    } else if (state.fractal_type == FRACTAL_NEWTON) {
      state.fractal_type = FRACTAL_MANDELBROT;
    }
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_h:
    state.screen_center = pixel_new_from_complex_plane_coordinates(&state, 0);
    state.complex_width = 3;
    state.max_iter = INITIAL_MAX_ITER;
    state.fractals_config.julia.z0 =
        pixel_new_from_complex_plane_coordinates(&state, INITIAL_JULIA_Z0);
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_w:
    state.fractals_config.julia.z0 =
        pixel_add_value(&state.fractals_config.julia.z0, 0.1 * I,
                        COORDINATES_TYPE_COMPLEX_PLANE);
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_s:
    state.fractals_config.julia.z0 =
        pixel_add_value(&state.fractals_config.julia.z0, -0.1 * I,
                        COORDINATES_TYPE_COMPLEX_PLANE);
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_a:
    state.fractals_config.julia.z0 = pixel_add_value(
        &state.fractals_config.julia.z0, -0.1, COORDINATES_TYPE_COMPLEX_PLANE);
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_d:
    state.fractals_config.julia.z0 = pixel_add_value(
        &state.fractals_config.julia.z0, +0.1, COORDINATES_TYPE_COMPLEX_PLANE);
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_o:
    state.show_overlays = !state.show_overlays;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_i:
    if (state.fractal_type == FRACTAL_NEWTON) {
      state.fractals_config.newton.iterations += 1;
    } else {
      state.max_iter += 10;
    }
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  case GDK_KEY_I:
    if (state.fractal_type == FRACTAL_NEWTON) {
      state.fractals_config.newton.iterations -= 1;
    } else {
      state.max_iter -= 10;
    }
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    break;
  }

  return TRUE;
}

Pixel initial_julia_z0;
unsigned int initial_root_index;
Pixel initial_root_position;

void on_drag_start(GtkGestureDrag *gesture, gdouble start_x, gdouble start_y,
                   gpointer user_data) {
  if (state.fractal_type == FRACTAL_JULIA) {
    initial_julia_z0 = state.fractals_config.julia.z0;
  } else if (state.fractal_type == FRACTAL_NEWTON) {
    Pixel mouse_position =
        pixel_new_from_screen_coordinates(&state, start_x + start_y * I);
    double complex mouse_position_complex =
        pixel_get_complex_plane_coordinates(&mouse_position);
    double min_distance = INFINITY;
    unsigned int min_index = 0;
    for (unsigned int i = 0; i < state.fractals_config.newton.num_roots; i++) {
      double distance =
          cabs(mouse_position_complex - state.fractals_config.newton.roots[i]);
      if (distance < min_distance) {
        min_distance = distance;
        min_index = i;
      }
    }

    initial_root_index = min_index;
    initial_root_position = pixel_new_from_complex_plane_coordinates(
        &state, state.fractals_config.newton.roots[initial_root_index]);
  }
}

void on_drag_update(GtkGestureDrag *gesture, gdouble offset_x, gdouble offset_y,
                    gpointer user_data) {

  if (state.fractal_type == FRACTAL_JULIA) {
    Pixel new_mouse_position = pixel_add_value(
        &initial_julia_z0, offset_x + offset_y * I, COORDINATES_TYPE_SCREEN);

    state.fractals_config.julia.z0 = new_mouse_position;
    gtk_widget_queue_draw(GTK_WIDGET(state.window->drawing_area));
  } else if (state.fractal_type == FRACTAL_NEWTON) {
    Pixel new_mouse_position =
        pixel_add_value(&initial_root_position, offset_x + offset_y * I,
                        COORDINATES_TYPE_SCREEN);

    state.fractals_config.newton.roots[initial_root_index] =
        pixel_get_complex_plane_coordinates(&new_mouse_position);
    gtk_widget_queue_draw(GTK_WIDGET(state.window->drawing_area));
  }
}

int main(int argc, char *argv[]) {

  double complex *roots = malloc(INITIAL_NEWTON_ROOTS * sizeof(double complex));
  for (int i = 0; i < INITIAL_NEWTON_ROOTS; i++) {
    roots[i] = cexp(2 * G_PI * I * i / INITIAL_NEWTON_ROOTS);
  }

  state = (State){
      .pixels = malloc(SIZE * SIZE * 3),
      .window = window_new("Fractales", SIZE, state.pixels, draw, on_key_press,
                           on_drag_start, on_drag_update),

      .fractal_type = FRACTAL_NEWTON,
      .fractals_config = {.mandelbrot = {},
                          .julia =
                              {
                                  .z0 =
                                      pixel_new_from_complex_plane_coordinates(
                                          &state, INITIAL_JULIA_Z0),
                              },
                          .newton = {.roots = roots,
                                     .num_roots = INITIAL_NEWTON_ROOTS,
                                     .iterations = INITIAL_NEWTON_ITERATIONS}},

      .max_iter = INITIAL_MAX_ITER,

      .complex_width = INITIAL_COMPLEX_WIDTH,
      .screen_center = pixel_new_from_complex_plane_coordinates(
          &state, INITIAL_SCREEN_CENTER_AS_COMPLEX),

      .show_overlays = true,
      .overlays_color = OVERLAYS_COLOR,
      .tick_step = 1,
  };

  int status = window_present(state.window);

  window_free(state.window);

  g_free(state.pixels);
  g_free(roots);

  return status;
}
