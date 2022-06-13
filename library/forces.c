#include "forces.h"
#include "body.h"
#include "collision.h"
#include "list.h"
#include "math.h"
#include "scene.h"
#include "vector.h"
#include <assert.h>
#include <stdlib.h>

const double MIN_DIST = 5.00;

struct aux {
  double c;
  list_t *bodies;
  collision_handler_t handler; // for regular forces, this is left null. This is
                               // only for collisions.
  void *aux;
  bool collided;
};

aux_t *aux_init(double c, list_t *bodies) {
  aux_t *new_aux = malloc(sizeof(aux_t));
  new_aux->c = c;
  new_aux->bodies = bodies;
  new_aux->handler = NULL;
  new_aux->aux = NULL;
  new_aux->collided = false;
  return new_aux;
}

void aux_set_handler(aux_t *aux, collision_handler_t handler) {
  aux->handler = handler;
}

collision_handler_t aux_get_handler(aux_t *aux) { return aux->handler; }

void aux_set_aux(aux_t *auxt, void *aux) { auxt->aux = aux; }

void *aux_get_aux(aux_t *aux) { return aux->aux; }

void free_aux(aux_t *auxt) {
  if (auxt->aux != NULL) {
    free(auxt->aux);
  }
  free(auxt);
}

void make_grav_force(void *aux) {
  body_t *body1 = list_get(((aux_t *)aux)->bodies, 0);
  body_t *body2 = list_get(((aux_t *)aux)->bodies, 1);
  vector_t distance =
      vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
  double distance_mag = sqrt(vec_dot(distance, distance));
  double force_mag;
  if (distance_mag < MIN_DIST) {
    distance_mag = MIN_DIST;
  }
  force_mag = ((aux_t *)aux)->c * body_get_mass(body1) * body_get_mass(body2) /
              pow(distance_mag, 2);
  vector_t force = vec_multiply(force_mag, vec_norm(distance));
  body_add_force(body1, force);
  body_add_force(body2, vec_negate(force));
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2) {
  list_t *bodies = list_init(2, (free_func_t)body_free);
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux_t *aux = aux_init(G, bodies);
  scene_add_bodies_force_creator(scene, (force_creator_t)make_grav_force,
                                 (void *)aux, bodies, (free_func_t)free_aux);
}

void make_spring_force(void *aux) {
  body_t *body1 = list_get(((aux_t *)aux)->bodies, 0);
  body_t *body2 = list_get(((aux_t *)aux)->bodies, 1);
  vector_t distance =
      vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
  vector_t force = vec_multiply(((aux_t *)aux)->c, distance);
  body_add_force(body1, force);
  body_add_force(body2, vec_negate(force));
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
  list_t *bodies = list_init(2, (free_func_t)body_free);
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux_t *aux = aux_init(k, bodies);
  scene_add_bodies_force_creator(scene, (force_creator_t)make_spring_force,
                                 (void *)aux, bodies, (free_func_t)free_aux);
}

void make_drag_force(void *aux) {
  body_t *body = list_get(((aux_t *)aux)->bodies, 0);
  vector_t force = vec_multiply(((aux_t *)aux)->c, body_get_velocity(body));
  body_add_force(body, vec_negate(force));
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
  list_t *bodies = list_init(1, (free_func_t)body_free);
  list_add(bodies, body);
  aux_t *aux = aux_init(gamma, bodies);
  scene_add_bodies_force_creator(scene, (force_creator_t)make_drag_force,
                                 (void *)aux, bodies, (free_func_t)free_aux);
}

void make_collision(void *aux) {
  aux_t *auxt = (aux_t *)aux;
  body_t *body1 = (body_t *)list_get(auxt->bodies, 0);
  body_t *body2 = (body_t *)list_get(auxt->bodies, 1);
  list_t *shape1 = body_get_shape(body1);
  list_t *shape2 = body_get_shape(body2);
  collision_info_t collision = find_collision(shape1, shape2);
  if (collision.collided) {
    if (!(auxt->collided)) {
      auxt->collided = true;
      (auxt->handler)(body1, body2, collision.axis, auxt->aux);
    }
  } else if (!(collision.collided)) {
    auxt->collided = false;
  }
  list_free(shape1);
  list_free(shape2);
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  list_t *bodies = list_init(2, (free_func_t)body_free);
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux_t *auxt = aux_init(0, bodies);
  auxt->aux = aux;
  auxt->handler = handler;
  scene_add_bodies_force_creator(scene, (force_creator_t)make_collision, auxt,
                                 bodies, (free_func_t)freer);
}

void make_destructive(body_t *body1, body_t *body2, vector_t axis, void *aux) {
  body_remove(body1);
  body_remove(body2);
}

void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  list_t *bodies = list_init(2, (free_func_t)body_free);
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux_t *auxt = aux_init(0, bodies);
  create_collision(scene, body1, body2, (collision_handler_t)make_destructive,
                   auxt, (free_func_t)free_aux);
}

void make_phys_collision(body_t *body1, body_t *body2, vector_t axis,
                         void *aux) {
  double comp_1 = vec_dot(vec_norm(axis), body_get_velocity(body1));
  double comp_2 = vec_dot(vec_norm(axis), body_get_velocity(body2));
  double mass1 = body_get_mass(body1);
  double mass2 = body_get_mass(body2);
  double reduced_mass = (mass1 * mass2) / (mass1 + mass2);
  if (mass1 == INFINITY) {
    reduced_mass = mass2;
  } else if (mass2 == INFINITY) {
    reduced_mass = mass1;
  }
  double impulse1_mag =
      reduced_mass * (1.0 + ((aux_t *)aux)->c) * (comp_2 - comp_1);
  double impulse2_mag =
      reduced_mass * (1.0 + ((aux_t *)aux)->c) * (comp_1 - comp_2);
  body_add_impulse(body1, vec_multiply(impulse1_mag, vec_norm(axis)));
  body_add_impulse(body2, vec_multiply(impulse2_mag, vec_norm(axis)));
}

void create_physics_collision(scene_t *scene, double elasticity, body_t *body1,
                              body_t *body2) {
  list_t *bodies = list_init(2, (free_func_t)body_free);
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux_t *new_aux = aux_init(elasticity, bodies);
  create_collision(scene, body1, body2, make_phys_collision, new_aux, NULL);
}

void make_phys_collision_remove_1(body_t *body1, body_t *body2, vector_t axis,
                                  void *aux) {
  double comp_1 = vec_dot(vec_norm(axis), body_get_velocity(body1));
  double comp_2 = vec_dot(vec_norm(axis), body_get_velocity(body2));
  double mass1 = body_get_mass(body1);
  double mass2 = body_get_mass(body2);
  double reduced_mass = (mass1 * mass2) / (mass1 + mass2);
  if (mass1 == INFINITY) {
    reduced_mass = mass2;
  } else if (mass2 == INFINITY) {
    reduced_mass = mass1;
  }
  double impulse1_mag =
      reduced_mass * (1.0 + ((aux_t *)aux)->c) * (comp_2 - comp_1);
  double impulse2_mag =
      reduced_mass * (1.0 + ((aux_t *)aux)->c) * (comp_1 - comp_2);
  body_add_impulse(body1, vec_multiply(impulse1_mag, vec_norm(axis)));
  body_add_impulse(body2, vec_multiply(impulse2_mag, vec_norm(axis)));
  body_remove(body1);
  // body_collide(body1);
  // body_collide(body2);
}

void create_physics_collision_remove_1(scene_t *scene, double elasticity,
                                       body_t *body1, body_t *body2) {
  list_t *bodies = list_init(2, (free_func_t)body_free);
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux_t *new_aux = aux_init(elasticity, bodies);
  create_collision(scene, body1, body2, make_phys_collision_remove_1, new_aux,
                   NULL);
}
