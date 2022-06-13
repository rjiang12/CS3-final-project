#include "body.h"
#include "color.h"
#include "list.h"
#include "polygon.h"
#include "vector.h"
#include <stdio.h>
#include <stdlib.h>

struct body {
  list_t *shape;
  double mass;
  rgb_color_t color;
  vector_t centroid;
  vector_t velocity;
  vector_t force;
  vector_t impulse;
  double angle;
  void *info;
  free_func_t freer;
  bool remove;
  // bool collided;
};

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  return body_init_with_info(shape, mass, color, NULL, NULL);
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer) {
  body_t *b = malloc(sizeof(body_t));
  b->shape = shape;
  b->mass = mass;
  b->color = color;
  b->centroid = polygon_centroid(b->shape);
  b->velocity = VEC_ZERO;
  b->angle = 0;
  b->force = VEC_ZERO;
  b->impulse = VEC_ZERO;
  b->info = info;
  b->freer = info_freer;
  b->remove = false;
  // b->collided = false;
  return b;
}

void body_free(body_t *body) {
  list_free(body->shape);
  free(body);
}

list_t *body_get_shape(body_t *body) {
  list_t *pointer = list_init(list_size(body->shape), free);
  for (int i = 0; i < list_size(body->shape); i++) {
    vector_t *vec = (vector_t *)list_get(body->shape, i);
    vector_t new_vec = vec_init((*vec).x, (*vec).y);
    vector_t *new_vec_p = malloc(sizeof(vector_t));
    *new_vec_p = new_vec;
    list_add(pointer, new_vec_p);
  }
  return pointer;
}

void body_set_shape(body_t *body, list_t *shape) {
  list_free(body->shape);
  body->shape = shape;
}

vector_t body_get_centroid(body_t *body) { return body->centroid; }

vector_t body_get_velocity(body_t *body) { return body->velocity; }

double body_get_mass(body_t *body) { return body->mass; }

rgb_color_t body_get_color(body_t *body) { return body->color; }

void *body_get_info(body_t *body) { return body->info; }

void body_add_info(body_t *body, void *new_info, free_func_t new_info_free) {
  if (body->info) {
    body->freer(body->info);
  }
  body->info = new_info;
  body->freer = new_info_free;
}

void body_set_centroid(body_t *body, vector_t x) {
  polygon_translate(body->shape, (vec_add(x, vec_negate(body->centroid))));
  body->centroid = x;
}

void body_set_velocity(body_t *body, vector_t v) { body->velocity = v; }

void body_set_rotation(body_t *body, double angle) {
  polygon_rotate(body->shape, (angle - body->angle), body->centroid);
  body->angle = angle;
}

void body_add_force(body_t *body, vector_t force) {
  body->force = vec_add(body->force, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
  body->impulse = vec_add(body->impulse, impulse);
}

void body_tick(body_t *body, double dt) {
  double impulse_x = body->impulse.x + dt * body->force.x;
  double impulse_y = body->impulse.y + dt * body->force.y;
  vector_t new_impulse = (vector_t){impulse_x, impulse_y};

  vector_t old_velocity = body->velocity;
  double velocity_x = body->velocity.x + (new_impulse.x) / (body->mass);
  double velocity_y = body->velocity.y + (new_impulse.y) / (body->mass);
  vector_t new_velocity = (vector_t){velocity_x, velocity_y};
  body_set_velocity(body, new_velocity);

  vector_t avg_velocity =
      vec_multiply(0.5, vec_add(old_velocity, new_velocity));
  vector_t new_centroid =
      vec_add(body->centroid, vec_multiply(dt, avg_velocity));
  body_set_centroid(body, new_centroid);

  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
}

void body_remove(body_t *body) { body->remove = true; }

bool body_is_removed(body_t *body) { return body->remove; }
