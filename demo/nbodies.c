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

const int NUM_PTS = 4;
const int NUM_BODIES = 40;
const int MAX_SIZE = 12;
const int MIN_SIZE = 6;
const int MIN_MASS = 50;
const int MAX_MASS = 150;
const double G = 75;
const double X_SIZE = 1000;
const double Y_SIZE = 500;
const double MAX_RGB = 255.0;

typedef struct state {
  scene_t *scene;
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

vector_t random_coor() {
  double rand_x = random_between(0, X_SIZE);
  double rand_y = random_between(0, Y_SIZE);
  vector_t coord = vec_init(rand_x, rand_y);
  return coord;
}

body_t *make_nbody(double radius, size_t num_points) {
  list_t *shape = list_init(num_points, free);
  for (int i = 0; i < num_points; i++) {
    vector_t vec = vec_init(radius, 0);
    double angle = i * 2 * M_PI / num_points;
    vec = vec_rotate(vec, angle);
    vector_t *vec_s = malloc(sizeof(vec));
    *vec_s = vec;
    list_add(shape, vec_s);
  }
  int mass = random_between(MIN_MASS, MAX_MASS);
  rgb_color_t color = rand_color();
  vector_t center = random_coor();
  body_t *body = body_init(shape, mass, color);
  body_set_centroid(body, center);
  return body;
}

void make_nbodies(int num_bodies, state_t *state) {
  for (int i = 0; i < num_bodies; i++) {
    body_t *nbody = make_nbody(random_between(MIN_SIZE, MAX_SIZE), NUM_PTS);
    list_add(scene_get_body_list(state->scene), nbody);
  }
}

state_t *emscripten_init() {
  srand(time(NULL));
  sdl_init(VEC_ZERO, (vector_t){X_SIZE, Y_SIZE});
  state_t *state = malloc(sizeof(state_t));
  state->scene = scene_init();
  make_nbodies(NUM_BODIES, state);
  for (int i = 0; i < list_size(scene_get_body_list(state->scene)); i++) {
    for (int j = i + 1; j < list_size(scene_get_body_list(state->scene)); j++) {
      body_t *body1 = scene_get_body(state->scene, i);
      body_t *body2 = scene_get_body(state->scene, j);
      create_newtonian_gravity(state->scene, G, body1, body2);
    }
  }
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
  free(state);
}
