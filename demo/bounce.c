#include "color.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// constants used in code for readability
const double BOUNCE_X_SIZE = 1000;
const double BOUNCE_Y_SIZE = 500;
const int NUM_STAR_POINTS = 5;
const double STAR_RADIUS = 120;
const double TURNING = 0.02;
const int POINTY = 3;

typedef struct state {
  // fields necessary for each frame to be drawn based on data stored in the
  // state
  vector_t *translation_vector;
  list_t *star_points;
} state_t;

const rgb_color_t RGB_VALUES = {0.5, 0.5, 0.5};
const vector_t ZERO_VEC = {0, 0};

/**
 * Generates a vector corresponding to a point of a regular star.
 * Helper function for *bounce_make_star().
 *
 * @param radius distance from center of star to point
 * @param i coefficient in numerator of fraction inside trig function
 * @param points number of points on the star
 * @param center where the center of the star is located
 * @param small_size 1 if an inner point should be generated, 0 if outer point
 * @return inner point of a star
 */
vector_t *bounce_make_point(double radius, int i, int points, vector_t center,
                            int small_size) {
  double num = 2 * M_PI * i / points;
  if (small_size) {
    num = num + (2 * M_PI) / (2 * points);
  }
  double x = radius * cos(num) + center.x;
  double y = radius * sin(num) + center.y;
  vector_t *pointer = malloc(sizeof(vector_t));
  vector_t v = {x, y};
  *pointer = v;
  return pointer;
}

/**
 * Generates a set of points corresponding to the vertices of a regular star.
 *
 * @param points number of points desired on the star whose points will be
 * generated
 * @param big_r desired distance from center of star to a point
 * @param center where the center of the star should be located
 * @return vec_list_t containing vertices of a regular star in counterclockwise
 * order
 */
list_t *bounce_make_star(size_t points, size_t big_r, vector_t center) {
  size_t small_r = big_r / POINTY;
  // list_t to populate must contain twice the number of points
  list_t *star = list_init(2 * points, free);
  // generate vectors two at a time; one "outer" point and one "inner" point
  for (int i = 0; i < (int)points; i++) {
    vector_t *big_point = bounce_make_point(big_r, i, points, center, 0);
    list_add(star, big_point);
    vector_t *small_point = bounce_make_point(small_r, i, points, center, 1);
    list_add(star, small_point);
  }
  return star;
}

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  state->translation_vector = malloc(sizeof(vector_t));
  vector_t *translation = malloc(sizeof(vector_t));
  *translation = (vector_t){1, 1};
  state->translation_vector = translation;
  state->star_points =
      bounce_make_star(NUM_STAR_POINTS, STAR_RADIUS,
                       (vector_t){BOUNCE_X_SIZE / 2, BOUNCE_Y_SIZE / 2});
  sdl_init(ZERO_VEC, (vector_t){BOUNCE_X_SIZE, BOUNCE_Y_SIZE});
  return state;
}

void emscripten_main(state_t *state) {
  // translate and rotate the star as per the "velocity" (translation vector)
  // and turning angle
  polygon_translate(state->star_points, state->translation_vector[0]);
  polygon_rotate(state->star_points, TURNING,
                 polygon_centroid(state->star_points));
  // with every run of emscripten_main(), check if any of the points touch a
  // boundary
  for (int i = 0; i < (int)list_size(state->star_points); i++) {
    // if x-boundary is touched, flip sign of x-component of translation vector
    // if y-boundary is touched, flip sign of y-component of translation vector
    if (((vector_t *)(list_get(state->star_points, (size_t)i)))->x <= 0 ||
        ((vector_t *)(list_get(state->star_points, (size_t)i)))->x >=
            BOUNCE_X_SIZE) {
      *state->translation_vector = (vector_t){-state->translation_vector[0].x,
                                              state->translation_vector[0].y};
    } else if (((vector_t *)(list_get(state->star_points, (size_t)i)))->y <=
                   0 ||
               ((vector_t *)(list_get(state->star_points, (size_t)i)))->y >=
                   BOUNCE_Y_SIZE) {
      *state->translation_vector = (vector_t){state->translation_vector[0].x,
                                              -state->translation_vector[0].y};
    }
  }
  // clear window, draw transformed star, then display the frame
  sdl_clear();
  sdl_draw_polygon(state->star_points, RGB_VALUES);
  sdl_show();
}

void emscripten_free(state_t *state) {
  free(state->translation_vector);
  list_free(state->star_points);
  free(state);
}
