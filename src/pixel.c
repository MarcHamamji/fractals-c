#include "pixel.h"
#include "state.h"
#include <complex.h>
#include <stdio.h>

double map(double x, double fromMin, double fromMax, double toMin,
                  double toMax) {
  return (x - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin;
}

double complex screen_to_complex_plane_coordinates(
    double complex coordinates, double complex screen_center_in_complex_plane,
    double complex width, int size) {

  double x = map(creal(coordinates), 0, size,
                 creal(screen_center_in_complex_plane) - width / 2,
                 creal(screen_center_in_complex_plane) + width / 2);
  double y = map(cimag(coordinates), 0, size,
                 cimag(screen_center_in_complex_plane) + width / 2,
                 cimag(screen_center_in_complex_plane) - width / 2);

  return x + y * I;
}
double complex complex_plane_to_screen_coordinates(
    double complex coordinates, double complex screen_center_in_complex_plane,
    double complex width, int size) {

  double x =
      map(creal(coordinates), creal(screen_center_in_complex_plane) - width / 2,
          creal(screen_center_in_complex_plane) + width / 2, 0, size);
  double y =
      map(cimag(coordinates), cimag(screen_center_in_complex_plane) - width / 2,
          cimag(screen_center_in_complex_plane) + width / 2, size, 0);

  return x + y * I;
}

Pixel pixel_new_from_complex_plane_coordinates(State *state,
                                               double complex coordinates) {
  Pixel pixel = (Pixel){
      ._state = state,
      ._complex_plane_coordinates = coordinates,
      ._complex_plane_coordinates_cached = true,
      ._screen_coordinates_cached = false,
  };

  return pixel;
}

Pixel pixel_new_from_screen_coordinates(State *state,
                                        double complex coordinates) {
  Pixel pixel = (Pixel){
      ._state = state,
      ._screen_coordinates = coordinates,
      ._screen_coordinates_cached = true,
      ._complex_plane_coordinates_cached = false,
  };

  return pixel;
}

double complex pixel_get_complex_plane_coordinates(Pixel *pixel) {

  if (pixel->_complex_plane_coordinates_cached) {
    return pixel->_complex_plane_coordinates;
  }

  pixel->_complex_plane_coordinates = screen_to_complex_plane_coordinates(
      pixel->_screen_coordinates,
      pixel_get_complex_plane_coordinates(&pixel->_state->screen_center),
      pixel->_state->complex_width, pixel->_state->window->size);

  pixel->_complex_plane_coordinates_cached = true;

  return pixel->_complex_plane_coordinates;
}

double complex pixel_get_screen_coordinates(Pixel *pixel) {

  if (pixel->_screen_coordinates_cached) {
    return pixel->_screen_coordinates;
  }

  pixel->_screen_coordinates = complex_plane_to_screen_coordinates(
      pixel->_complex_plane_coordinates,
      pixel_get_complex_plane_coordinates(&pixel->_state->screen_center),
      pixel->_state->complex_width, pixel->_state->window->size);

  pixel->_screen_coordinates_cached = true;

  return pixel->_screen_coordinates;
}

Pixel pixel_add(Pixel *pixel1, Pixel *pixel2) {
  if (pixel1->_screen_coordinates_cached && pixel2->_screen_coordinates_cached)
    return pixel_new_from_screen_coordinates(pixel1->_state,
                                             pixel1->_screen_coordinates +
                                                 pixel2->_screen_coordinates);

  else if (pixel1->_complex_plane_coordinates_cached &&
           pixel2->_complex_plane_coordinates_cached)
    return pixel_new_from_complex_plane_coordinates(
        pixel1->_state, pixel1->_complex_plane_coordinates +
                            pixel2->_complex_plane_coordinates);

  else if (pixel1->_screen_coordinates_cached &&
           pixel2->_complex_plane_coordinates_cached) {
    return pixel_new_from_complex_plane_coordinates(
        pixel1->_state, pixel_get_complex_plane_coordinates(pixel1) +
                            pixel2->_complex_plane_coordinates);
  }

  else {
    return pixel_new_from_complex_plane_coordinates(
        pixel1->_state, pixel1->_complex_plane_coordinates +
                            pixel_get_complex_plane_coordinates(pixel2));
  }
}

Pixel pixel_add_value(Pixel *pixel, double complex value,
                      COORDINATES_TYPE type) {
  switch (type) {
  case COORDINATES_TYPE_SCREEN:
    return pixel_new_from_screen_coordinates(
        pixel->_state, pixel_get_screen_coordinates(pixel) + value);
    break;
  case COORDINATES_TYPE_COMPLEX_PLANE:
    return pixel_new_from_complex_plane_coordinates(
        pixel->_state, pixel_get_complex_plane_coordinates(pixel) + value);
    break;
  }

  return pixel_new_from_complex_plane_coordinates(pixel->_state, 0);
}

char *pixel_string(Pixel *pixel, COORDINATES_TYPE type) {
  double complex coordinates;

  switch (type) {
  case COORDINATES_TYPE_COMPLEX_PLANE:
    coordinates = pixel_get_complex_plane_coordinates(pixel);
    break;
  case COORDINATES_TYPE_SCREEN:
    coordinates = pixel_get_screen_coordinates(pixel);
    break;
  default:
    coordinates = 0;
  }

  char *string = malloc(128 * sizeof(char));

  sprintf(string, "%.2f + %.2fi", creal(coordinates), cimag(coordinates));
  return string;
}
