#pragma once

#include "cairo.h"
#include "state.h"

void draw_axes(cairo_t *cr, State *state);

void draw_julia_z0(cairo_t *cr, State *state);

void draw_labels(cairo_t *cr, State *state);

void draw_overlays(cairo_t *cr, State *state);
