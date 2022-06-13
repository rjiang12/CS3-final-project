#include "body.h"
#include "color.h"
#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int NUM_VERTICES = 1000;
const double X_SIZE = 1000;
const double Y_SIZE = 500;
const int MASS = 100;
const int NUM_CIRCLES = 20;
const int RADIUS = 1000 / 40;
const double K1 = 50;
const double GAMMA1 = 2;
const double K2 = 10;
const double GAMMA2 = 30;
const double PORTION_DAMPENED = (1.0 / 3.0);
const double VERTICAL_SHIFT_1 = 0.75 * Y_SIZE;
const double AMPLITUDE_1 = Y_SIZE / 4.0;
const double VERTICAL_SHIFT_2 = 0.60 * Y_SIZE;
const double AMPLITUDE_2 = Y_SIZE / 10.0;
const double PHASE_FACTOR = 3 * M_PI;
const double MAX_RGB = 255.0;
const rgb_color_t BLACK = (rgb_color_t){0, 0, 0};

typedef struct state {
  scene_t *scene;
  list_t *centers;
} state_t;

int random_between(int min, int max) {
  return ((rand() % (max - min + 1)) + min);
}

rgb_color_t rand_color() {
  float rand_r = random_between(0, MAX_RGB) / MAX_RGB;
  float rand_g = random_between(0, MAX_RGB) / MAX_RGB;
  float rand_b = random_between(0, MAX_RGB) / MAX_RGB;
  rgb_color_t color = (rgb_color_t){rand_r, rand_g, rand_b};
  return color;
}

list_t *make_circle_shape(double radius) {
  list_t *shape = list_init(NUM_VERTICES, (free_func_t)free);
  for (int i = 0; i < NUM_VERTICES; i++) {
    vector_t vec = vec_init(radius, 0);
    double angle = i * 2 * M_PI / NUM_VERTICES;
    vec = vec_rotate(vec, angle);
    vector_t *vec_s = malloc(sizeof(vec));
    *vec_s = vec;
    list_add(shape, vec_s);
  }
  return shape;
}

vector_t find_initial(vector_t initial_pos, int i, int num_circles,
                      double radius) {
  initial_pos.x += 2 * radius;
  if (i <= num_circles / 3) {
    initial_pos.y = VERTICAL_SHIFT_1 +
                    AMPLITUDE_1 * cos(PHASE_FACTOR * i / num_circles) - radius;

  } else if (i > 2 * num_circles / 3) {
    initial_pos.y = VERTICAL_SHIFT_2 -
                    AMPLITUDE_2 * cos(PHASE_FACTOR * i / num_circles) - radius;
  } else {
    initial_pos.y = Y_SIZE / 2.0 - radius;
  }
  return initial_pos;
}

void make_circles(scene_t *scene, int num_circles, double radius) {
  vector_t initial_pos = vec_negate((vector_t){radius, 0});
  for (int i = 0; i < num_circles; i++) {
    body_t *body = body_init(make_circle_shape(radius), MASS, rand_color());
    initial_pos = find_initial(initial_pos, i, num_circles, radius);
    body_set_centroid(body, initial_pos);
    scene_add_body(scene, body);
  }
}

void spring_and_drag(scene_t *scene, body_t *body, body_t *center, double k,
                     double gamma) {
  create_spring(scene, k, body, center);
  create_drag(scene, gamma, body);
}

void apply_forces(scene_t *scene, list_t *centers, double k1, double gamma1,
                  double k2, double gamma2, double dampened) {
  for (int i = 0; i < scene_bodies(scene); i++) {
    list_t *center_shape = make_circle_shape(RADIUS);
    body_t *center = body_init(center_shape, INFINITY, BLACK);
    body_t *body = scene_get_body(scene, i);
    body_set_centroid(center, (vector_t){body_get_centroid(body).x,
                                         Y_SIZE / 2.0 - RADIUS / 2.0});
    list_add(centers, center);
    double k_val;
    double gamma_val;
    if (i >= scene_bodies(scene) * dampened) {
      k_val = k2;
      gamma_val = gamma2;
    } else {
      k_val = k1;
      gamma_val = gamma1;
    }
    spring_and_drag(scene, body, center, k_val, gamma_val);
  }
}

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  sdl_init(VEC_ZERO, (vector_t){X_SIZE, Y_SIZE});
  state->scene = scene_init();
  state->centers = list_init(NUM_CIRCLES, (free_func_t)body_free);
  make_circles(state->scene, RADIUS, NUM_CIRCLES);
  apply_forces(state->scene, state->centers, K1, GAMMA1, K2, GAMMA2,
               PORTION_DAMPENED);
  sdl_render_scene(state->scene);
  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  scene_tick(state->scene, dt);
  sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  list_free(state->centers);
  free(state);
}
