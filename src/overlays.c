#include "overlays.h"

void draw_axes(cairo_t *cr, State *state) {
  cairo_set_source_rgb(cr, 0.5, 1, 0.8);
  cairo_set_line_width(cr, 2);

  Pixel origin = pixel_new_from_complex_plane_coordinates(state, 0);
  double complex screen_origin = pixel_get_screen_coordinates(&origin);

  cairo_move_to(cr, creal(screen_origin), 0);
  cairo_line_to(cr, creal(screen_origin), state->window->size);
  cairo_move_to(cr, 0, cimag(screen_origin));
  cairo_line_to(cr, state->window->size, cimag(screen_origin));

  cairo_stroke(cr);

  cairo_set_source_rgb(cr, 0.5, 1, 0.8);
  cairo_set_font_size(cr, 16);

  cairo_move_to(cr, creal(screen_origin) + 10, cimag(screen_origin) + 20);
  cairo_show_text(cr, "0");
  cairo_move_to(cr, state->window->size - 30, cimag(screen_origin) + 20);
  cairo_show_text(cr, "Re");
  cairo_move_to(cr, creal(screen_origin) + 10, 20);
  cairo_show_text(cr, "Im");
  cairo_move_to(cr, creal(screen_origin) + 10, state->window->size - 10);
  cairo_show_text(cr, "0");
}

void draw_julia_z0(cairo_t *cr, State *state) {
  cairo_set_source_rgb(cr, 0.5, 1, 0.8);
  cairo_set_line_width(cr, 2);

  double complex z0 = pixel_get_screen_coordinates(&state->julia_z0);

  cairo_arc(cr, creal(z0), cimag(z0), 5, 0, 2 * M_PI);
  cairo_fill(cr);

  cairo_set_font_size(cr, 16);
  cairo_move_to(cr, creal(z0) + 10, cimag(z0) + 6);

  char *label = pixel_string(&state->julia_z0, COORDINATES_TYPE_COMPLEX_PLANE);
  cairo_show_text(cr, label);

  free(label);
}

void draw_labels(cairo_t *cr, State *state) {
  cairo_set_source_rgb(cr, 0.5, 1, 0.8);
  cairo_set_font_size(cr, 16);

  if (state->julia) {
    cairo_move_to(cr, 10, 20);
    char *label =
        pixel_string(&state->julia_z0, COORDINATES_TYPE_COMPLEX_PLANE);
    cairo_show_text(cr, "c = ");
    cairo_move_to(cr, 36, 20);
    cairo_show_text(cr, label);

    cairo_move_to(cr, 10, 40);
    cairo_show_text(cr, "z0 = variable");
    free(label);
  } else {
    cairo_move_to(cr, 10, 20);
    cairo_show_text(cr, "c = variable");

    cairo_move_to(cr, 10, 40);
    cairo_show_text(cr, "z0 = 0");
  }

  cairo_move_to(cr, 10, 60);
  cairo_show_text(cr, "max iterations = ");
  char max_iter_label[5];
  sprintf(max_iter_label, "%d", state->max_iter);
  cairo_move_to(cr, 136, 60);
  cairo_show_text(cr, max_iter_label);
}

void draw_overlays(cairo_t *cr, State *state) {

  draw_axes(cr, state);
  draw_labels(cr, state);

  if (state->julia) {
    draw_julia_z0(cr, state);
  }
}
