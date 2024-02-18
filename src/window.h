#include <gtk/gtk.h>

typedef struct {
  unsigned int size;
  guchar *pixels;
  GtkApplication *app;
  GtkApplicationWindow *app_window;
  GtkDrawingArea *drawing_area;
  GtkEventController *event_controller;
  GtkGesture *drag_gesture;
} Window;

Window *window_new(
    char *name, unsigned int size, guchar *data,
    void (*draw)(GtkDrawingArea *drawing_area, cairo_t *cr, int width,
                 int height, gpointer _data),
    gboolean (*on_key_press)(GtkEventControllerKey *controller, guint keyval,
                             guint keycode, GdkModifierType state),
    void (*on_drag_start)(GtkGestureDrag *gesture, gdouble offset_x,
                          gdouble offset_y, gpointer _data),
    void (*on_drag_update)(GtkGestureDrag *gesture, gdouble offset_x,
                           gdouble offset_y, gpointer _data)

);

int window_present(Window *window);

void window_free(Window *window);
