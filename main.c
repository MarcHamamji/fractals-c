#include <cairo.h>
#include <complex.h>
#include <gtk/gtk.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 800

bool julia = false;
double complex julia_z0 = 0 + 0.5 * I;

int max_iter = 40;
double complex_width = 3;
double complex complex_screen_center = 0;

GtkWidget *drawing_area;

guchar *data;

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
  double complex complex_point =
      screen_to_complex(x + y * I, complex_screen_center, complex_width);
  int8_t threshold;

  if (julia) {
    threshold = diverging_threshold(complex_point, julia_z0, max_iter);
  } else {
    threshold = diverging_threshold(0, complex_point, max_iter);
  }

  double color;
  if (threshold == -1) {
    color = 0;
  } else {
    color = threshold * 255.0 / 20;
    if (color > 255)
      color = 255;
  }

  data[(y * width + x) * 3] = color;
  data[(y * width + x) * 3 + 1] = color;
  data[(y * width + x) * 3 + 2] = color;
  if (!julia && 2 * cimag(complex_screen_center) - y < height &&
      2 * cimag(complex_screen_center) - y >= 0) {
    data[(int)((2 * cimag(complex_screen_center) - y) * width + x) * 3] = color;
    data[(int)((2 * cimag(complex_screen_center) - y) * width + x) * 3 + 1] =
        color;
    data[(int)((2 * cimag(complex_screen_center) - y) * width + x) * 3 + 2] =
        color;
  }
}

static void draw_axes(cairo_t *cr) {
  cairo_set_source_rgb(cr, 0.5, 1, 0.8);
  cairo_set_line_width(cr, 2);

  double complex screen_center =
      complex_to_screen(0, complex_screen_center, complex_width);
  cairo_move_to(cr, creal(screen_center), 0);
  cairo_line_to(cr, creal(screen_center), SIZE);
  cairo_move_to(cr, 0, cimag(screen_center));
  cairo_line_to(cr, SIZE, cimag(screen_center));

  cairo_stroke(cr);
}

static void draw_julia_z0(cairo_t *cr) {
  cairo_set_source_rgb(cr, 0.5, 1, 0.8);
  cairo_set_line_width(cr, 2);

  double complex z0 =
      complex_to_screen(julia_z0, complex_screen_center, complex_width);

  cairo_arc(cr, creal(z0), cimag(z0), 5, 0, 2 * M_PI);
  cairo_fill(cr);

  cairo_set_font_size(cr, 16);
  cairo_move_to(cr, creal(z0) + 10, cimag(z0) + 10);
  char *label =
      g_strdup_printf("%.1f + %.1fi", creal(julia_z0), cimag(julia_z0));
  cairo_show_text(cr, label);
}

static void draw(GtkDrawingArea *drawing_area, cairo_t *cr, int width,
                 int height, gpointer _data) {

  if (!julia) {
    if (cimag(complex_screen_center) > height / 2.0) {
      for (int x = 0; x < width; x++) {
        for (int y = 0; y <= cimag(complex_screen_center); y++) {
          color_point(cr, x, y, width, height);
        }
      }
    } else {
      for (int x = 0; x < width; x++) {
        for (int y = cimag(complex_screen_center); y < height; y++) {
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

  gdk_cairo_set_source_pixbuf(cr,
                              gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB,
                                                       FALSE, 8, width, height,
                                                       width * 3, NULL, NULL),
                              0, 0);
  cairo_paint(cr);

  draw_axes(cr);
  if (julia) {
    draw_julia_z0(cr);
  }
}

static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval,
                               guint keycode, GdkModifierType state) {

  switch (keyval) {
  case GDK_KEY_Up:
    complex_screen_center += 0.1 * complex_width * I;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_Down:
    complex_screen_center -= 0.1 * complex_width * I;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_Right:
    complex_screen_center += 0.1 * complex_width;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_Left:
    complex_screen_center -= 0.1 * complex_width;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_plus:
  case GDK_KEY_equal:
    complex_width *= 0.9;
    max_iter += 2;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_minus:
    complex_width *= 1.1;
    max_iter -= 2;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_q:
    free(data);
    exit(0);
  case GDK_KEY_j:
    julia = !julia;
    gtk_widget_queue_draw(drawing_area);
    break;
  case GDK_KEY_h:
    complex_screen_center = 0;
    complex_width = 3;
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

  data = malloc(SIZE * SIZE * 3);

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Fractales");
  gtk_window_set_default_size(GTK_WINDOW(window), SIZE, SIZE);
  gtk_window_set_resizable(GTK_WINDOW(window), false);

  drawing_area = gtk_drawing_area_new();
  gtk_window_set_child(GTK_WINDOW(window), drawing_area);
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw, NULL,
                                 NULL);

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
