#include <gtk/gtk.h>

typedef struct {
  unsigned int size;
  guchar *data;
  GtkApplication *app;
  GtkWidget *app_window;
  GtkWidget *drawing_area;
  GtkEventController *event_controller;
} Window;

Window *window_new(char *name, unsigned int size, guchar *data,
                   void (*draw)(GtkDrawingArea *drawing_area, cairo_t *cr,
                                int width, int height, gpointer _data),
                   gboolean (*on_key_press)(GtkEventControllerKey *controller,
                                            guint keyval, guint keycode,
                                            GdkModifierType state)

);

int window_present(Window *window);

void window_destroy(Window *window);