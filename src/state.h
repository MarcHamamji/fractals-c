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
} State;
