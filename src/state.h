#pragma once

#include <complex.h>
#include <gtk/gtk.h>

#include "pixel.h"
#include "window.h"

typedef struct State {
  guchar *pixels;
  Window *window;

  bool julia;
  Pixel julia_z0;

  int max_iter;

  double complex_width;
  Pixel screen_center;

  bool show_overlays;
  float overlays_color[3];
  double tick_step;
} State;
