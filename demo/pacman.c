#include "pacman.h"
#include "sdl_wrapper.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const double X_SIZE = 1000;
const double Y_SIZE = 500;
const double X_ORIGIN = 0;
const double Y_ORIGIN = 0;
const rgb_color_t RGB_YELLOW = {1, 1, 0};
const rgb_color_t RGB_GRAY = {0.5, 0.5, 0.5};
const vector_t UP_INIT = {0, 10};
const vector_t DOWN_INIT = {0, -10};
const vector_t LEFT_INIT = {-10, 0};
const vector_t RIGHT_INIT = {10, 0};
const vector_t UP_ACCEL = {0, 10};
const vector_t DOWN_ACCEL = {0, -10};
const vector_t LEFT_ACCEL = {-10, 0};
const vector_t RIGHT_ACCEL = {10, 0};
const double PACMAN_MASS = 10;
const double PELLET_MASS = 1;
const int PACMAN_RADIUS = 60;
const int PELLET_RADIUS = 5;
const int PELLET_SIDES = 4;

typedef struct state {
  scene_t *scene;
  size_t time_passed; // essentially, time passed since last pellet spawned.
  body_t *pacbody;
} state_t;

typedef int (*comparator_t)(double a);

int neg_comp(double velocity_component) { return velocity_component < 0; }

int pos_comp(double velocity_component) { return velocity_component > 0; }

body_t *make_pacman(double mass, rgb_color_t color, vector_t center,
                    double size) {
  // make a pacman shape centered at the given center with a radius of size (as
  // a list_t) initialize the body using the list_t made above and the arguments
  // provided return the initialized body
  assert(center.x < X_SIZE && center.x > X_ORIGIN);
  assert(center.y < Y_SIZE && center.y > Y_ORIGIN);
  list_t *shape = list_init(52, free);
  vector_t *central = malloc(sizeof(vector_t));
  *central = VEC_ZERO;
  list_add(shape, central);
  for (int i = 6; i < 58; i++) {
    vector_t vec = vec_init(size, 0);
    double angle = 2 * M_PI * i / 64;
    vec = vec_rotate(vec, angle);
    vector_t *vec_s = malloc(sizeof(vec));
    *vec_s = vec;
    list_add(shape, vec_s);
  }
  body_t *pacbody = body_init(shape, mass, color);
  body_set_centroid(pacbody, center);
  return pacbody;
}

body_t *make_pellet(double mass, rgb_color_t color, vector_t center,
                    double size) {
  // make ONE pellet at the given center using the given attributes
  // initialize and return a body constituting that pellet
  assert(center.x <= X_SIZE && center.x >= X_ORIGIN);
  assert(center.y <= Y_SIZE && center.y >= Y_ORIGIN);
  list_t *shape = list_init(PELLET_SIDES, free);
  for (int i = 0; i < PELLET_SIDES; i++) {
    vector_t vec = vec_init(size, 0);
    double angle = i * 2 * M_PI / PELLET_SIDES;
    vec = vec_rotate(vec, angle);
    vector_t *vec_s = malloc(sizeof(vec));
    *vec_s = vec;
    list_add(shape, vec_s);
  }
  body_t *pellet = body_init(shape, mass, color);
  body_set_centroid(pellet, center);
  return pellet;
}

void eat_pellet(state_t *state) {
  for (int i = 0; i < scene_bodies(state->scene); i++) {
    body_t *pellet = scene_get_body(state->scene, i);
    if ((fabs(body_get_centroid(pellet).x -
              body_get_centroid(state->pacbody).x) < PACMAN_RADIUS) &&
        (fabs(body_get_centroid(pellet).y -
              body_get_centroid(state->pacbody).y) < PACMAN_RADIUS)) {
      scene_remove_body(state->scene, i);
    }
  }
}

void wrap_around(state_t *state) {
  if (X_SIZE - body_get_centroid(state->pacbody).x < -PACMAN_RADIUS) {
    vector_t old_centroid = body_get_centroid(state->pacbody);
    body_set_centroid(state->pacbody,
                      (vector_t){PACMAN_RADIUS, old_centroid.y});
  } else if (0 - body_get_centroid(state->pacbody).x > PACMAN_RADIUS) {
    vector_t old_centroid = body_get_centroid(state->pacbody);
    body_set_centroid(state->pacbody,
                      (vector_t){X_SIZE - PACMAN_RADIUS, old_centroid.y});
  } else if (Y_SIZE - body_get_centroid(state->pacbody).y < -PACMAN_RADIUS) {
    vector_t old_centroid = body_get_centroid(state->pacbody);
    body_set_centroid(state->pacbody,
                      (vector_t){old_centroid.x, PACMAN_RADIUS});
  } else if (0 - body_get_centroid(state->pacbody).y > PACMAN_RADIUS) {
    vector_t old_centroid = body_get_centroid(state->pacbody);
    body_set_centroid(state->pacbody,
                      (vector_t){old_centroid.x, Y_SIZE - PACMAN_RADIUS});
  }
}

void key_handler_func_helper(double dt, double velocity_component, body_t *body,
                             vector_t acceleration, vector_t init_vel,
                             double angle, comparator_t comparison) {
  body_set_rotation(body, angle);
  if (comparison(velocity_component)) {
    vector_t new_velocity =
        vec_add(body_get_velocity(body), vec_multiply(dt, acceleration));
    body_set_velocity(body, new_velocity);
  } else {
    body_set_velocity(body, init_vel);
  }
}

void key_handler_func(state_t *state, char key_pressed,
                      key_event_type_t event_type, double dt) {
  if (event_type == KEY_PRESSED) {
    switch (key_pressed) {
    case RIGHT_ARROW:
      key_handler_func_helper(dt, body_get_velocity(state->pacbody).x,
                              state->pacbody, RIGHT_ACCEL, RIGHT_INIT, 0,
                              pos_comp);
      break;
    case LEFT_ARROW:
      key_handler_func_helper(dt, body_get_velocity(state->pacbody).x,
                              state->pacbody, LEFT_ACCEL, LEFT_INIT, M_PI,
                              neg_comp);
      break;
    case UP_ARROW:
      key_handler_func_helper(dt, body_get_velocity(state->pacbody).y,
                              state->pacbody, UP_ACCEL, UP_INIT, (M_PI / 2),
                              pos_comp);
      break;
    case DOWN_ARROW:
      key_handler_func_helper(dt, body_get_velocity(state->pacbody).y,
                              state->pacbody, DOWN_ACCEL, DOWN_INIT,
                              (3 * M_PI / 2), neg_comp);
      break;
    default:
      break;
    }
  } else if (event_type == KEY_RELEASED) {
    if (body_get_velocity(state->pacbody).x > 0) {
      body_set_velocity(state->pacbody, RIGHT_INIT);
    } else if (body_get_velocity(state->pacbody).x < 0) {
      body_set_velocity(state->pacbody, LEFT_INIT);
    } else if (body_get_velocity(state->pacbody).y > 0) {
      body_set_velocity(state->pacbody, UP_INIT);
    } else if (body_get_velocity(state->pacbody).y < 0) {
      body_set_velocity(state->pacbody, DOWN_INIT);
    }
  }
}

void add_pellet(state_t *state) {
  scene_add_body(state->scene, make_pellet(PELLET_MASS, RGB_GRAY,
                                           (vector_t){rand() % (int)X_SIZE,
                                                      rand() % (int)Y_SIZE},
                                           PELLET_RADIUS));
}

state_t *emscripten_init() {
  srand(time(NULL));
  sdl_init(VEC_ZERO, (vector_t){X_SIZE, Y_SIZE});
  state_t *state = malloc(sizeof(state_t));
  state->scene = scene_init();
  state->time_passed = 0.0;
  state->pacbody =
      make_pacman(PACMAN_MASS, RGB_YELLOW, (vector_t){X_SIZE / 2, Y_SIZE / 2},
                  PACMAN_RADIUS);
  for (int i = 0; i < 25; i++) {
    add_pellet(state);
  }
  sdl_on_key((key_handler_t)key_handler_func);
  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  scene_tick(state->scene, dt);
  body_tick(state->pacbody, dt);
  if (dt > 0) {
    state->time_passed += 1;
    if (state->time_passed >= 100) {
      add_pellet(state);
      state->time_passed = 0.0;
    }
  }
  eat_pellet(state);
  wrap_around(state);
  eat_pellet(state);
  printf("x_vel: %f y_vel: %f \n", body_get_velocity(state->pacbody).x,
         body_get_velocity(state->pacbody).y);
  printf("x_cent: %f y_cent: %f \n", body_get_centroid(state->pacbody).x,
         body_get_centroid(state->pacbody).y);
  sdl_render_scene_pacman(state->scene, state->pacbody);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
