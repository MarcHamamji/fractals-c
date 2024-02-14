#include "cairo.h"
#include <complex.h>
#include <gtk/gtk.h>
#include <stdint.h>
#include <stdio.h>

const int size = 800;

bool julia = false;
double complex julia_z0 = 0 + 0.5 * I;

int max_iter = 40;
double scale = 400;
double complex center = size * 0.7 + (size / 2.0) * I;

GtkWidget *drawing_area;

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

double complex mapped_coordinates(double complex position,
                                  double complex center, double scale) {
  return conj((position - center) / scale);
}

double complex reverse_mapped_coordinates(double complex position,
                                          double complex center, double scale) {
  return conj(position) * scale + center;
}

void color_point(cairo_t *cr, int x, int y, int width, int height) {
  double complex complex_coords = mapped_coordinates(x + y * I, center, scale);
  int8_t threshold;
  if (julia) {
    threshold = diverging_threshold(complex_coords, julia_z0, max_iter);
  } else {
    threshold = diverging_threshold(0, complex_coords, max_iter);
  }
  if (threshold == -1) {
    cairo_set_source_rgb(cr, 0, 0, 0);
  } else {
    double color = threshold / 20.0;
    cairo_set_source_rgb(cr, color, color, color);
  }
  cairo_rectangle(cr, x, y, 1, 1);
  if (!julia && 2 * cimag(center) - y < height && 2 * cimag(center) - y >= 0) {
    cairo_rectangle(cr, x, 2 * cimag(center) - y, 1, 1);
  }
  cairo_fill(cr);
}

static void draw_axes(cairo_t *cr) {
  cairo_set_source_rgb(cr, 0.5, 1, 0.8);
  cairo_set_line_width(cr, 2);
  cairo_move_to(cr, creal(center), 0);
  cairo_line_to(cr, creal(center), size);
  cairo_move_to(cr, 0, cimag(center));
  cairo_line_to(cr, size, cimag(center));
  cairo_stroke(cr);
}

static void draw_julia_z0(cairo_t *cr) {
  cairo_set_source_rgb(cr, 0.5, 1, 0.8);
  cairo_set_line_width(cr, 2);
  double complex z0 = reverse_mapped_coordinates(julia_z0, center, scale);
  cairo_arc(cr, creal(z0), cimag(z0), 5, 0, 2 * M_PI);
  cairo_fill(cr);

  cairo_set_font_size(cr, 16);
  cairo_move_to(cr, creal(z0) + 10, cimag(z0) + 10);
  char *label =
      g_strdup_printf("%.1f + %.1fi", creal(julia_z0), cimag(julia_z0));
  cairo_show_text(cr, label);
}

static void draw(GtkDrawingArea *drawing_area, cairo_t *cr, int width,
                          int height, gpointer data) {

  if (!julia) {
    if (cimag(center) > height / 2.0) {
      for (int x = 0; x < width; x++) {
        for (int y = 0; y <= cimag(center); y++) {
          color_point(cr, x, y, width, height);
        }
      }
    } else {
      for (int x = 0; x < width; x++) {
        for (int y = cimag(center); y < height; y++) {
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

  draw_axes(cr);
  if (julia) {
    draw_julia_z0(cr);
  }
}


static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval,
                               guint keycode, GdkModifierType state) {

  switch (keyval) {
  case GDK_KEY_Up:
    center += 50 * I;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_Down:
    center -= 50 * I;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_Right:
    center -= 50;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_Left:
    center += 50;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_plus:
  case GDK_KEY_equal: {
    scale += 50;
    max_iter += 1;
    gtk_widget_queue_draw(drawing_area);
    break;
  }
  case GDK_KEY_minus:
    scale -= 50;
    max_iter -= 1;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_q:
    exit(0);
  case GDK_KEY_j:
    julia = !julia;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_h:
    center = size * 0.5 + (size * 0.5) * I;
    scale = 400;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_w:
    julia_z0 += 0.1 * I;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_s:
    julia_z0 -= 0.1 * I;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_a:
    julia_z0 -= 0.1;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_d:
    julia_z0 += 0.1;
    gtk_widget_queue_draw(drawing_area);
    break;
  }

  return TRUE;
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Fractales");
  gtk_window_set_default_size(GTK_WINDOW(window), size, size);
  gtk_window_set_resizable(GTK_WINDOW(window), false);

  drawing_area = gtk_drawing_area_new();
  gtk_window_set_child(GTK_WINDOW(window), drawing_area);
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw,
                                 NULL, NULL);

  GtkEventController *event_controller;
  event_controller = gtk_event_controller_key_new();

  g_signal_connect_object(event_controller, "key-pressed",
                          G_CALLBACK(on_key_pressed), drawing_area,
                          G_CONNECT_SWAPPED);

  gtk_widget_add_controller(GTK_WIDGET(window), event_controller);

  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app = gtk_application_new("com.github.marchamamji.fractales",
                            G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
