#include "overlays.h"
#include "cairo.h"
#include "pixel.h"

static void set_overlay_colors(cairo_t *cr, State *state) {
  cairo_set_source_rgb(cr, state->overlays_color[0], state->overlays_color[1],
                       state->overlays_color[2]);
}

void draw_axes(cairo_t *cr, State *state) {
  set_overlay_colors(cr, state);
  cairo_set_line_width(cr, 2);

  Pixel origin = pixel_new_from_complex_plane_coordinates(state, 0);
  double complex screen_origin = pixel_get_screen_coordinates(&origin);

  cairo_move_to(cr, creal(screen_origin), 0);
  cairo_line_to(cr, creal(screen_origin), state->window->size);
  cairo_move_to(cr, 0, cimag(screen_origin));
  cairo_line_to(cr, state->window->size, cimag(screen_origin));

  cairo_stroke(cr);

  cairo_set_font_size(cr, 16);

  cairo_move_to(cr, creal(screen_origin) + 10, cimag(screen_origin) + 20);
  cairo_show_text(cr, "0");
  cairo_move_to(cr, state->window->size - 30, cimag(screen_origin) + 20);
  cairo_show_text(cr, "Re");
  cairo_move_to(cr, creal(screen_origin) + 10, 20);
  cairo_show_text(cr, "Im");

  double density = 3;
  state->tick_step = pow(10, floor(log10(state->complex_width / density)));
  draw_axes_ticks(cr, state);
}

void draw_axes_ticks(cairo_t *cr, State *state) {
  set_overlay_colors(cr, state);
  cairo_set_line_width(cr, 2);
  cairo_set_font_size(cr, 12);

  Pixel top_left = pixel_new_from_screen_coordinates(state, 0);
  double complex top_left_complex =
      pixel_get_complex_plane_coordinates(&top_left);

  double left_start =
      creal(top_left_complex) - fmod(creal(top_left_complex), state->tick_step);
  double top_start =
      cimag(top_left_complex) - fmod(cimag(top_left_complex), state->tick_step);

  for (double i = left_start; i < left_start + state->complex_width;
       i += state->tick_step) {
    if (i == 0.0) {
      continue;
    }

    Pixel tick = pixel_new_from_complex_plane_coordinates(state, i);
    double complex tick_screen = pixel_get_screen_coordinates(&tick);
    cairo_move_to(cr, creal(tick_screen), cimag(tick_screen) - 5);
    cairo_line_to(cr, creal(tick_screen), cimag(tick_screen) + 5);
  }

  for (double i = top_start; i > top_start - state->complex_width;
       i -= state->tick_step) {
    if (i == 0.0) {
      continue;
    }
    Pixel tick = pixel_new_from_complex_plane_coordinates(state, I * i);
    double complex tick_screen = pixel_get_screen_coordinates(&tick);
    cairo_move_to(cr, creal(tick_screen) - 5, cimag(tick_screen));
    cairo_line_to(cr, creal(tick_screen) + 5, cimag(tick_screen));
  }

  cairo_stroke(cr);
}

void draw_julia_z0(cairo_t *cr, State *state) {
  set_overlay_colors(cr, state);
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
  set_overlay_colors(cr, state);
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
    cairo_show_text(cr, "z0 = 0 + 0i");
  }

  cairo_move_to(cr, 10, 60);
  cairo_show_text(cr, "max iterations = ");
  char max_iter_label[8];
  sprintf(max_iter_label, "%d", state->max_iter);
  cairo_move_to(cr, 136, 60);
  cairo_show_text(cr, max_iter_label);

  cairo_move_to(cr, 10, 80);
  cairo_show_text(cr, "graduation = ");
  char tick_step_label[16];
  sprintf(tick_step_label, "%g", state->tick_step);
  cairo_move_to(cr, 112, 80);
  cairo_show_text(cr, tick_step_label);
}

void draw_scale(cairo_t *cr, State *state) {

  set_overlay_colors(cr, state);
  cairo_set_line_width(cr, 2);

  cairo_move_to(cr, state->window->size - 150, state->window->size - 10);
  cairo_line_to(cr, state->window->size - 10, state->window->size - 10);
  cairo_move_to(cr, state->window->size - 150, state->window->size - 10);
  cairo_line_to(cr, state->window->size - 150, state->window->size - 20);
  cairo_move_to(cr, state->window->size - 10, state->window->size - 10);
  cairo_line_to(cr, state->window->size - 10, state->window->size - 20);
  cairo_stroke(cr);

  double complex_equivalent =
      map(140, 0, state->window->size, 0, state->complex_width);
  cairo_move_to(cr, state->window->size - 126, state->window->size - 20);

  char scale[16];
  sprintf(scale, "%g", complex_equivalent);
  cairo_show_text(cr, scale);
}

void draw_cross(cairo_t *cr, State *state) {
  set_overlay_colors(cr, state);
  cairo_set_line_width(cr, 2);
  // draw a 10 px cross at the center of the window
  cairo_move_to(cr, state->window->size / 2.0 - 10, state->window->size / 2.0);
  cairo_line_to(cr, state->window->size / 2.0 + 10, state->window->size / 2.0);
  cairo_move_to(cr, state->window->size / 2.0, state->window->size / 2.0 - 10);
  cairo_line_to(cr, state->window->size / 2.0, state->window->size / 2.0 + 10);

  cairo_stroke(cr);
}

void draw_overlays(cairo_t *cr, State *state) {

  draw_axes(cr, state);
  draw_labels(cr, state);
  draw_scale(cr, state);
  draw_cross(cr, state);

  if (state->julia) {
    draw_julia_z0(cr, state);
  }
}
