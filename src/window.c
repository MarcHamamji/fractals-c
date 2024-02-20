#include "window.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  Window *window;
  char *name;
  unsigned int size;
  guchar *pixels;
  void (*draw)(GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height,
               gpointer _data);
  gboolean (*on_key_press)(GtkEventControllerKey *controller, guint keyval,
                           guint keycode, GdkModifierType state);
  void (*on_drag_update)(GtkGestureDrag *gesture, gdouble offset_x,
                         gdouble offset_y, gpointer _data);
  void (*on_drag_start)(GtkGestureDrag *gesture, gdouble offset_x,
                        gdouble offset_y, gpointer _data);
} WindowActivationParams;

static void activate(GtkApplication *app, WindowActivationParams *params) {
  Window *window = params->window;

  window->app_window = GTK_APPLICATION_WINDOW(gtk_application_window_new(app));

  gtk_window_set_title(GTK_WINDOW(window->app_window), params->name);
  gtk_window_set_default_size(GTK_WINDOW(window->app_window), params->size,
                              params->size + 36);
  gtk_window_set_resizable(GTK_WINDOW(window->app_window), false);

  window->drawing_area = GTK_DRAWING_AREA(gtk_drawing_area_new());
  gtk_window_set_child(GTK_WINDOW(window->app_window),
                       GTK_WIDGET(window->drawing_area));

  gtk_drawing_area_set_draw_func(window->drawing_area, params->draw,
                                 window->pixels, NULL);

  window->event_controller = gtk_event_controller_key_new();
  g_signal_connect_object(window->event_controller, "key-pressed",
                          G_CALLBACK(params->on_key_press),
                          window->drawing_area, G_CONNECT_SWAPPED);

  gtk_widget_add_controller(GTK_WIDGET(window->app_window),
                            window->event_controller);

  window->drag_gesture = gtk_gesture_drag_new();
  g_signal_connect(window->drag_gesture, "drag-update",
                   G_CALLBACK(params->on_drag_update), window->drawing_area);
  g_signal_connect(window->drag_gesture, "drag-begin",
                   G_CALLBACK(params->on_drag_start), window->drawing_area);
  gtk_widget_add_controller(GTK_WIDGET(window->drawing_area),
                            GTK_EVENT_CONTROLLER(window->drag_gesture));

  gtk_window_present(GTK_WINDOW(window->app_window));

  free(params);
}

Window *window_new(
    char *name, unsigned int size, guchar *pixels,
    void (*draw)(GtkDrawingArea *drawing_area, cairo_t *cr, int width,
                 int height, gpointer _data),
    gboolean (*on_key_press)(GtkEventControllerKey *controller, guint keyval,
                             guint keycode, GdkModifierType state),
    void (*on_drag_start)(GtkGestureDrag *gesture, gdouble offset_x,
                          gdouble offset_y, gpointer _data),
    void (*on_drag_update)(GtkGestureDrag *gesture, gdouble offset_x,
                           gdouble offset_y, gpointer _data)) {

  WindowActivationParams *params = malloc(sizeof(WindowActivationParams));
  *params = (WindowActivationParams){
      .window = malloc(sizeof(Window)),
      .name = name,
      .size = size,
      .pixels = pixels,
      .draw = draw,
      .on_key_press = on_key_press,
      .on_drag_update = on_drag_update,
      .on_drag_start = on_drag_start,
  };

  Window *window = params->window;

  window->app = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(window->app, "activate", G_CALLBACK(activate),
                   (gpointer)params);

  window->size = size;
  window->pixels = pixels;

  return window;
}

int window_present(Window *window) {
  int status = g_application_run(G_APPLICATION(window->app), 0, NULL);
  return status;
}

void window_free(Window *window) {
  // g_object_unref(window->app);
}
