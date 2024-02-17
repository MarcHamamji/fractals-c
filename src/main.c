#include <cairo.h>
#include <complex.h>
#include <gtk/gtk.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "window.h"

#define SIZE 800

#define INITIAL_JULIA false;
#define INITIAL_JULIA_Z0 0 + 0.5 * I;

#define INITIAL_MAX_ITER 40;
#define INITIAL_COMPLEX_WIDTH 3;
#define INITIAL_SCREEN_CENTER_AS_COMPLEX 0;

typedef struct {
  guchar *data;
  Window *window;

  bool julia;
  double complex julia_z0;

  int max_iter;

  double complex_width;
  double complex screen_center_as_complex;
} State;

State state;

static double map(double x, double fromMin, double fromMax, double toMin,
                  double toMax) {
  return (x - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin;
}

static double complex complex_to_screen(double complex z, double complex center,
                                        double width) {
  double x = map(creal(z), creal(center) - width / 2, creal(center) + width / 2,
                 0, SIZE);
  double y = map(cimag(z), cimag(center) - width / 2, cimag(center) + width / 2,
                 SIZE, 0);
  return x + I * y;
}

static double complex screen_to_complex(double complex z, double complex center,
                                        double width) {
  double x = map(creal(z), 0, SIZE, creal(center) - width / 2,
                 creal(center) + width / 2);
  double y = map(cimag(z), 0, SIZE, cimag(center) + width / 2,
                 cimag(center) - width / 2);
  return x + I * y;
}

static int8_t diverging_threshold(double complex initial_z, double complex c,
                                  int max_iter) {
  double complex z = initial_z;
  for (int i = 0; i < max_iter; i++) {
    z = z * z + c;
    if (cabs(z) > 2) {
      return i;
    }
  }
  return -1;
}

void color_point(cairo_t *cr, int x, int y, int width, int height) {
  double complex complex_point = screen_to_complex(
      x + y * I, state.screen_center_as_complex, state.complex_width);
  int8_t threshold;

  if (state.julia) {
    threshold =
        diverging_threshold(complex_point, state.julia_z0, state.max_iter);
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

  state.data[(y * width + x) * 3] = color;
  state.data[(y * width + x) * 3 + 1] = color;
  state.data[(y * width + x) * 3 + 2] = color;
  if (!state.julia && 2 * cimag(state.screen_center_as_complex) - y < height &&
      2 * cimag(state.screen_center_as_complex) - y >= 0) {
    state.data[(int)((2 * cimag(state.screen_center_as_complex) - y) * width +
                     x) *
               3] = color;
    state.data[(int)((2 * cimag(state.screen_center_as_complex) - y) * width +
                     x) *
                   3 +
               1] = color;
    state.data[(int)((2 * cimag(state.screen_center_as_complex) - y) * width +
                     x) *
                   3 +
               2] = color;
  }
}

static void draw_axes(cairo_t *cr) {
  cairo_set_source_rgb(cr, 0.5, 1, 0.8);
  cairo_set_line_width(cr, 2);

  double complex screen_center =
      complex_to_screen(0, state.screen_center_as_complex, state.complex_width);
  cairo_move_to(cr, creal(screen_center), 0);
  cairo_line_to(cr, creal(screen_center), SIZE);
  cairo_move_to(cr, 0, cimag(screen_center));
  cairo_line_to(cr, SIZE, cimag(screen_center));

  cairo_stroke(cr);
}

static void draw_julia_z0(cairo_t *cr) {
  cairo_set_source_rgb(cr, 0.5, 1, 0.8);
  cairo_set_line_width(cr, 2);

  double complex z0 = complex_to_screen(
      state.julia_z0, state.screen_center_as_complex, state.complex_width);

  cairo_arc(cr, creal(z0), cimag(z0), 5, 0, 2 * M_PI);
  cairo_fill(cr);

  cairo_set_font_size(cr, 16);
  cairo_move_to(cr, creal(z0) + 10, cimag(z0) + 10);
  char *label = g_strdup_printf("%.1f + %.1fi", creal(state.julia_z0),
                                cimag(state.julia_z0));
  cairo_show_text(cr, label);
}

static void draw(GtkDrawingArea *drawing_area, cairo_t *cr, int width,
                 int height, gpointer _data) {

  if (!state.julia) {
    if (cimag(state.screen_center_as_complex) > height / 2.0) {
      for (int x = 0; x < width; x++) {
        for (int y = 0; y <= cimag(state.screen_center_as_complex); y++) {
          color_point(cr, x, y, width, height);
        }
      }
    } else {
      for (int x = 0; x < width; x++) {
        for (int y = cimag(state.screen_center_as_complex); y < height; y++) {
          color_point(cr, x, y, width, height);
        }
      }
    }
  } else {
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
        color_point(cr, x, y, width, height);
      }
    }
  }

  gdk_cairo_set_source_pixbuf(
      cr,
      gdk_pixbuf_new_from_data(state.data, GDK_COLORSPACE_RGB, FALSE, 8, width,
                               height, width * 3, NULL, NULL),
      0, 0);
  cairo_paint(cr);

  draw_axes(cr);
  if (state.julia) {
    draw_julia_z0(cr);
  }
}

static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval,
                               guint keycode, GdkModifierType _state) {

  GtkWidget *drawing_area = state.window->drawing_area;

  switch (keyval) {
  case GDK_KEY_Up:
    state.screen_center_as_complex += 0.1 * state.complex_width * I;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_Down:
    state.screen_center_as_complex -= 0.1 * state.complex_width * I;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_Right:
    state.screen_center_as_complex += 0.1 * state.complex_width;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_Left:
    state.screen_center_as_complex -= 0.1 * state.complex_width;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_plus:
  case GDK_KEY_equal:
    state.complex_width *= 0.9;
    state.max_iter += 2;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_minus:
    state.complex_width *= 1.1;
    state.max_iter -= 2;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_q:
    window_destroy(state.window);
    break;
  case GDK_KEY_j:
    state.julia = !state.julia;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_h:
    state.screen_center_as_complex = 0;
    state.complex_width = 3;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_w:
    state.julia_z0 += 0.1 * I;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_s:
    state.julia_z0 -= 0.1 * I;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_a:
    state.julia_z0 -= 0.1;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_d:
    state.julia_z0 += 0.1;
    gtk_widget_queue_draw(drawing_area);
    break;
  }

  return TRUE;
}

int main(int argc, char *argv[]) {
  state.data = malloc(SIZE * SIZE * 3);

  state.window =
      window_new("Fractales", SIZE, state.data, draw, on_key_pressed);

  state.julia = INITIAL_JULIA;
  state.julia_z0 = INITIAL_JULIA_Z0;
  state.max_iter = INITIAL_MAX_ITER;
  state.complex_width = INITIAL_COMPLEX_WIDTH;
  state.screen_center_as_complex = INITIAL_SCREEN_CENTER_AS_COMPLEX;

  int status = window_present(state.window);

  free(state.data);

  return status;
}
