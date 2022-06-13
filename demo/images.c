#include <sdl_wrapper.h>
#include <scene.h>
#include "vector.h"
#include "list.h"
#include "body.h"
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

SDL_Surface *PUCK; 
size_t CIRCLE_SIDES = 25; 

typedef struct state {
  scene_t *scene;
} state_t;

body_t *make_circle(double mass, rgb_color_t color, vector_t center, double size, void *info) {
  // makes a ball at given using the given attributes
  // initialize and return a body constituting that ball
  list_t *shape = list_init(CIRCLE_SIDES, free);
  for (int i = 0; i < CIRCLE_SIDES; i++) {
    vector_t vec = vec_init(size, 0);
    double angle = i * 2 * M_PI / CIRCLE_SIDES;
    vec = vec_rotate(vec, angle);
    vector_t *vec_s = malloc(sizeof(vec));
    *vec_s = vec;
    list_add(shape, vec_s);
  }
  body_t *circle =
      body_init_with_info(shape, mass, color, (void *)info, free);
  body_set_centroid(circle, center);
  return circle;
}

void initialize_puck(state_t *state) {
  body_t *puck = make_circle(100, (rgb_color_t) {1.0, 0, 0}, (vector_t){(1200 / 2), (800 / 2) + 100}, 25, NULL);
  scene_add_body(state->scene, puck);
}

state_t *emscripten_init() {
  srand(time(NULL));
  sdl_init(VEC_ZERO, (vector_t){1000.0, 1000.0});
  state_t *state = malloc(sizeof(state_t));
  state->scene = scene_init();
  PUCK = IMG_Load("assets/puck.png");
  if(PUCK == NULL) {
    printf("PUCK IS NULL \n"); 
  }
  initialize_puck(state); 
  return state; 
}

void emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  render_img(PUCK, scene_get_body(state->scene, 0), 25); 
  scene_tick(state->scene, dt); 
  //sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state) {
  SDL_FreeSurface(PUCK); 
  free(state); 
}