#pragma once

#include <complex.h>
#include <stdbool.h>

typedef struct State State;

typedef struct {
    State* _state;

    double complex _complex_plane_coordinates;
    double complex _screen_coordinates;

    bool _complex_plane_coordinates_cached;
    bool _screen_coordinates_cached;
} Pixel;

#include "state.h"

typedef enum {
    COORDINATES_TYPE_COMPLEX_PLANE = 0,
    COORDINATES_TYPE_SCREEN = 1,
} COORDINATES_TYPE;

double map(double x,
           double fromMin,
           double fromMax,
           double toMin,
           double toMax);

double complex screen_to_complex_plane_coordinates(
    double complex coordinates,
    double complex screen_center_in_complex_plane,
    double complex width,
    int size);

double complex complex_plane_to_screen_coordinates(
    double complex coordinates,
    double complex screen_center_in_complex_plane,
    double complex width,
    int size);

Pixel pixel_new_from_complex_plane_coordinates(State* state,
                                               double complex coordinates);

Pixel pixel_new_from_screen_coordinates(State* state,
                                        double complex coordinates);

double complex pixel_get_complex_plane_coordinates(Pixel* pixel);

double complex pixel_get_screen_coordinates(Pixel* pixel);

Pixel pixel_add(Pixel* pixel1, Pixel* pixel2);

Pixel pixel_add_value(Pixel* pixel,
                      double complex value,
                      COORDINATES_TYPE type);

void pixel_string(Pixel* pixel, COORDINATES_TYPE type, char* output);
