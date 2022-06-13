#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// constants used in code for readability
const double G_X_SIZE = 1000;
const double G_Y_SIZE = 500;
const double GROUND_HEIGHT = 20;
const int NUM_STAR_POINTS = 5;
const double G_X_ORIGIN = 0;
const double G_Y_ORIGIN = 0;

const double STAR_RADIUS = 50;
const double TURNING = 0.02;
const int POINTINESS = 3;
const int MAX_NUM_POINTS = 10;
const int MIN_NUM_POINTS = 3;
const double VELOCITY_DECAY = -0.85;
const vector_t TRANSLATION = {0.4, -0.2};
const vector_t GRAVITY = {0, -.01};
static double TIME = 0;
const double WAIT_TIME = 3;

typedef struct state {
  // fields necessary for each frame to be drawn based on data stored in the
  // state
  list_t *star_list;
  list_t *ground;
} state_t;

const rgb_color_t GRAY = {0.5, 0.5, 0.5};

typedef struct star {
  vector_t translation_vector;
  list_t *star_points;
  rgb_color_t color;
  double num_points;
  double size;
} star_t;

struct list {
  void **s;
  int size;
  int capacity;
};

/**
 * Generates a vector corresponding to a point of a regular star.
 * Helper function for *make_star().
 *
 * @param radius distance from center of star to point
 * @param i coefficient in numerator of fraction inside trig function
 * @param points number of points on the star
 * @param center where the center of the star is located
 * @param small_size 1 if an inner point should be generated, 0 if outer point
 * @return inner point of a star
 */
vector_t *make_point(double radius, int i, int points, vector_t center,
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
 * @return list_t containing vertices of a regular star in counterclockwise
 * order
 */
list_t *make_star(size_t points, size_t big_r, vector_t center) {
  size_t small_r = big_r / POINTINESS;
  // list_t to populate must contain twice the number of points
  list_t *star = list_init(2 * points, free);
  // generate vectors two at a time; one "outer" point and one "inner" point
  for (int i = 0; i < (int)points; i++) {
    vector_t *big_point = make_point(big_r, i, points, center, 0);
    list_add(star, big_point);
    vector_t *small_point = make_point(small_r, i, points, center, 1);
    list_add(star, small_point);
  }
  return star;
}

vector_t *make_pointer(double x, double y) {
  vector_t *pointer = malloc(sizeof(vector_t));
  vector_t v = {x, y};
  *pointer = v;
  return pointer;
}

list_t *make_ground() {
  list_t *ground = list_init((size_t)4, free);
  list_add(ground, make_pointer(0, 0));
  list_add(ground, make_pointer(G_X_SIZE, 0));
  list_add(ground, make_pointer(G_X_SIZE, GROUND_HEIGHT));
  list_add(ground, make_pointer(0, GROUND_HEIGHT));
  return ground;
}

star_t *star_init(double num_points, double size, rgb_color_t color) {
  star_t *star = malloc(sizeof(star_t));
  star->translation_vector = TRANSLATION;
  star->color = color;
  star->num_points = num_points;
  star->size = size;
  // begin displaying stars in the top-left corner
  vector_t start_center = {G_X_ORIGIN + size, G_Y_SIZE - size};
  star->star_points = make_star((size_t)num_points, (size_t)size, start_center);
  return star;
}

/**
 * Generates a random color by generating random RGB values.
 *
 * @return rgb_color_t containing randomly-generated RGB values
 */
rgb_color_t RGB_generator() {
  srandom(time(NULL)); // initialize random seed w/ current time
  rgb_color_t colors = {rand() / (float)RAND_MAX, rand() / (float)RAND_MAX,
                        rand() / (float)RAND_MAX};
  return colors;
}

state_t *emscripten_init() {
  srand(time(NULL));
  state_t *state = malloc(sizeof(state_t));
  state->star_list = list_init(1, free);
  star_t *star = star_init(((rand() % MAX_NUM_POINTS) + MIN_NUM_POINTS),
                           STAR_RADIUS, RGB_generator());
  list_add(state->star_list, star);
  state->ground = make_ground();
  sdl_init(VEC_ZERO, (vector_t){G_X_SIZE, G_Y_SIZE});
  TIME = TIME + time_since_last_tick();
  return state;
}

void emscripten_main(state_t *state) {
  srand(time(NULL));
  TIME = TIME + time_since_last_tick();
  // if a certain amount of time has passed, add a new star to the scene
  if (TIME > WAIT_TIME) {
    TIME = 0;
    star_t *star = star_init(((rand() % MAX_NUM_POINTS) + MIN_NUM_POINTS),
                             STAR_RADIUS, RGB_generator());
    list_add(state->star_list, star);
  }
  // with every run of emscripten_main(), check if any of the points touch the y
  // boundary
  for (int i = 0; i < (int)list_size(state->star_list); i++) {
    for (int j = 0;
         j <
         (int)list_size(((star_t *)list_get(state->star_list, i))->star_points);
         j++) {
      // if y-boundary is touched, flip sign of y-component of translation
      // vector and multiply by a fractional scalar to reduce initial upward
      // velocity (inelastic collision w/ground)
      if (((vector_t *)(list_get(
               ((star_t *)list_get(state->star_list, i))->star_points, j)))
              ->y < GROUND_HEIGHT) {
        ((star_t *)list_get(state->star_list, (size_t)i))->translation_vector =
            (vector_t){((star_t *)list_get(state->star_list, (size_t)i))
                           ->translation_vector.x,
                       ((star_t *)list_get(state->star_list, (size_t)i))
                               ->translation_vector.y *
                           VELOCITY_DECAY};
        while (
            polygon_centroid(
                ((star_t *)list_get(state->star_list, (size_t)i))->star_points)
                .y < ((star_t *)list_get(state->star_list, (size_t)i))->size +
                         GROUND_HEIGHT) {
          polygon_translate(
              ((star_t *)list_get(state->star_list, (size_t)i))->star_points,
              ((star_t *)list_get(state->star_list, (size_t)i))
                  ->translation_vector);
        }
      }
    }
  }
  // translate and rotate the star as per the "velocity" (translation vector)
  // and turning angle; also adjust the translation vector by adding gravity to
  // it
  for (int i = 0; i < state->star_list->size; i++) {
    polygon_translate(
        ((star_t *)list_get(state->star_list, (size_t)i))->star_points,
        ((star_t *)list_get(state->star_list, (size_t)i))->translation_vector);
    polygon_rotate(
        ((star_t *)list_get(state->star_list, (size_t)i))->star_points, TURNING,
        polygon_centroid(
            ((star_t *)list_get(state->star_list, (size_t)i))->star_points));
    ((star_t *)list_get(state->star_list, (size_t)i))->translation_vector =
        vec_add(((star_t *)list_get(state->star_list, (size_t)i))
                    ->translation_vector,
                GRAVITY);
  }
  // with every run of emscripten_main(), check if any stars have left the frame
  for (int i = 0; i < (int)list_size(state->star_list); i++) {
    if (polygon_centroid(
            ((star_t *)list_get(state->star_list, (size_t)i))->star_points)
                .x -
            G_X_SIZE >
        ((star_t *)list_get(state->star_list, (size_t)i))->size) {
      free(list_remove(state->star_list, 0));
    }
  }
  sdl_clear();
  // clear window, draw transformed star, then display the frame
  for (int i = 0; i < (int)list_size(state->star_list); i++) {
    sdl_draw_polygon(
        ((star_t *)list_get(state->star_list, (size_t)i))->star_points,
        ((star_t *)list_get(state->star_list, (size_t)i))->color);
  }
  // draw ground
  sdl_draw_polygon(state->ground, GRAY);
  sdl_show();
}

void emscripten_free(state_t *state) {
  for (int i = 0; i < (int)list_size(state->star_list); i++) {
    free(((star_t *)list_get(state->star_list, (size_t)i))->star_points);
    free(((star_t *)list_get(state->star_list, (size_t)i)));
  }
  free(state->star_list);
  free(state->ground);
  free(state);
}
