#include "scene.h"
#include "body.h"
#include "forces.h"
#include "list.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

int const INITIAL_BODIES = 50;
int const INITIAL_FORCES = 3;

struct scene {
  list_t *bodies;
  list_t *forces;
};

struct force {
  force_creator_t creator;
  void *aux;
  free_func_t freer;
  list_t *bodies;
};

force_t *force_init(force_creator_t creator, void *aux, free_func_t freer,
                    list_t *bodies) {
  force_t *force = malloc(sizeof(force_t));
  force->creator = creator;
  force->aux = aux;
  force->freer = freer;
  force->bodies = bodies;
  return force;
}

void free_force(force_t *force) {
  if (force->freer != NULL) {
    force->freer(force->aux);
  }
  free(force);
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  list_free(scene->forces);
  free(scene);
}

scene_t *scene_init(void) {
  scene_t *scene = malloc(sizeof(scene_t));
  scene->bodies = list_init(INITIAL_BODIES, (free_func_t)body_free);
  scene->forces = list_init(INITIAL_FORCES, (free_func_t)free_force);
  return scene;
}

size_t scene_bodies(scene_t *scene) { return list_size(scene->bodies); }

body_t *scene_get_body(scene_t *scene, size_t index) {
  assert((index >= 0) && (index < list_size(scene->bodies)));
  return list_get(scene->bodies, index);
}

list_t *scene_get_body_list(scene_t *scene) {
  list_t *pointer = scene->bodies;
  return pointer;
}

list_t *scene_get_forces(scene_t *scene) {
  list_t *pointer = scene->forces;
  return pointer;
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
}

void scene_remove_body(scene_t *scene, size_t index) {
  body_t *removed = scene_get_body(scene, index);
  body_remove(removed);
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
  scene_add_bodies_force_creator(scene, forcer, aux, NULL, freer);
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer) {
  force_t *force = force_init(forcer, aux, freer, bodies);
  list_add(scene->forces, force);
}

bool force_is_removed(body_t *body, force_t *force) {
  list_t *f_bodies = force->bodies;
  for (int i = 0; i < list_size(f_bodies); i++) {
    if ((body_t *)list_get(f_bodies, i) == body) {
      return true;
    }
  }
  return false;
}

void scene_tick(scene_t *scene, double dt) {
  // update forces
  for (int i = 0; i < list_size(scene->forces); i++) {
    force_t *force = list_get(scene->forces, i);
    force_creator_t creator = force->creator;
    assert(creator != NULL);
    creator(force->aux);
  }
  // remove bodies as necessary
  int idx = 0;
  while (idx < list_size(scene->bodies)) {
    body_t *body = (body_t *)list_get(scene->bodies, idx);
    if (body_is_removed(body)) {
      int force_idx = 0;
      while (force_idx < list_size(scene->forces)) {
        if (force_is_removed(body,
                             (force_t *)list_get(scene->forces, force_idx))) {
          free_force(list_remove(scene->forces, force_idx));
        } else {
          force_idx++;
        }
      }
      body_t *removed = scene_get_body(scene, idx);
      list_remove(scene->bodies, idx);
      body_free(removed);

    } else if (!body_is_removed(body)) {
      idx++;
    }
  }
  for (int i = 0; i < list_size(scene->bodies); i++) {
    body_tick(scene_get_body(scene, i), dt);
  }
}
