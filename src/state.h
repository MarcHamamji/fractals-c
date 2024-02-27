#pragma once

#include <complex.h>
#include <gtk/gtk.h>

#include "pixel.h"
#include "window.h"

typedef enum {
  FRACTAL_MANDELBROT,
  FRACTAL_JULIA,
} FRACTAL_TYPE;

typedef struct {
  char _placeholder;
} MandelbrotConfig;

typedef struct {
  Pixel z0;
} JuliaConfig;

typedef struct {
  MandelbrotConfig mandelbrot;
  JuliaConfig julia;
} FractalsConfig;

typedef struct State {
  guchar *pixels;
  Window *window;

  FRACTAL_TYPE fractal_type;
  FractalsConfig fractals_config;

  int max_iter;

  double complex_width;
  Pixel screen_center;

  bool show_overlays;
  float overlays_color[3];
  double tick_step;
} State;
