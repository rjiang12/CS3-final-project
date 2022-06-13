#include "body.h"
#include "collision.h"
#include "sdl_wrapper.h"
#include "time.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const double X_SIZE = 1200;
const double Y_SIZE = 1200;
const double X_ORIGIN = 0;
const double Y_ORIGIN = 0;
const rgb_color_t RGB_GRAY = {0.5, 0.5, 0.5};
const rgb_color_t RGB_GREEN = {0.1, 0.8, 0.1};
const rgb_color_t RGB_BLACK = {0.0, 0.0, 0.0};
const double MASS = 5.0;
const double ALIEN_RADIUS = 50.0;
const double ALIEN_SPACING = 5.0;
const int SIDES_NUM = 64;
const int ALIEN_START_CUT = 4;
const int ALIEN_END_CUT = 28;
const int ALIEN_COLS = 7;
const vector_t LEFT_VEL = {-400, 0};
const vector_t RIGHT_VEL = {400, 0};
const vector_t ALIEN_VEL = {100, 0};
const double SEMIMAJOR = 30.0;
const double SEMIMINOR = 10.0;
const double PLAYER_LEVEL = 0.1;
const double BULLET_MASS = 1;
const int BULLET_RADIUS = 5;
const int BULLET_SIDES = 4;
static double ALIEN_TIME = 0;
static double PLAYER_TIME = 0;
const double ALIEN_WAIT_TIME = 3e-3;
const double PLAYER_WAIT_TIME = 1e-4;
const vector_t BULLET_VELOCITY = {0, 500};
const double ALIEN_RADIUS_MULTIPLIER = 5.0;

char *PLAYER_INFO = "p";
char *ALIEN_INFO = "a";
char *PLAYER_BULLET_INFO = "pb";
char *ALIEN_BULLET_INFO = "ab";

typedef struct state {
  scene_t *scene;
} state_t;

body_t *make_alien(double mass, rgb_color_t color, vector_t center,
                   double size) {
  // make an alien centered at the given center with a radius of size (as
  // a list_t) initialize the body using the list_t made above and the arguments
  // provided return the initialized body
  assert(center.x < X_SIZE && center.x > X_ORIGIN);
  assert(center.y < Y_SIZE && center.y > Y_ORIGIN);
  list_t *shape = list_init(ALIEN_END_CUT - ALIEN_START_CUT, free);
  vector_t *central = malloc(sizeof(vector_t));
  *central = VEC_ZERO;
  list_add(shape, central);
  for (int i = ALIEN_START_CUT; i < ALIEN_END_CUT; i++) {
    vector_t vec = vec_init(size, 0);
    double angle = 2 * M_PI * i / SIDES_NUM;
    vec = vec_rotate(vec, angle);
    vector_t *vec_s = malloc(sizeof(vec));
    *vec_s = vec;
    list_add(shape, vec_s);
  }
  body_t *alien_body =
      body_init_with_info(shape, mass, color, (void *)ALIEN_INFO, free);
  body_set_centroid(alien_body, center);
  body_set_velocity(alien_body, ALIEN_VEL);
  return alien_body;
}

body_t *make_player(double mass, rgb_color_t color, vector_t center) {
  // makes elliptical player centered at the given center
  // (as a list_t) initialize the body using the list_t made above and the
  // arguments provided return the initialized body
  assert(center.x < X_SIZE && center.x > X_ORIGIN);
  assert(center.y < Y_SIZE && center.y > Y_ORIGIN);
  list_t *shape = list_init(SIDES_NUM, free);
  vector_t *central = malloc(sizeof(vector_t));
  *central = VEC_ZERO;
  list_add(shape, central);
  for (int i = 0; i <= SIDES_NUM; i++) {
    vector_t vec = {SEMIMAJOR * cos(2 * M_PI * i / SIDES_NUM),
                    SEMIMINOR * sin(2 * M_PI * i / SIDES_NUM)};
    vector_t *vec_s = malloc(sizeof(vec));
    *vec_s = vec;
    list_add(shape, vec_s);
  }
  body_t *player_body =
      body_init_with_info(shape, mass, color, (void *)PLAYER_INFO, free);
  body_set_centroid(player_body, center);
  return player_body;
}

body_t *make_bullet(double mass, rgb_color_t color, vector_t center,
                    double size, char *bullet_info) {
  // makes a bullet at given using the given attributes
  // initialize and return a body constituting that bullet
  list_t *shape = list_init(BULLET_SIDES, free);
  for (int i = 0; i < BULLET_SIDES; i++) {
    vector_t vec = vec_init(size, 0);
    double angle = i * 2 * M_PI / BULLET_SIDES;
    vec = vec_rotate(vec, angle);
    vector_t *vec_s = malloc(sizeof(vec));
    *vec_s = vec;
    list_add(shape, vec_s);
  }
  body_t *bullet =
      body_init_with_info(shape, mass, color, (void *)bullet_info, free);
  body_set_centroid(bullet, center);
  if (bullet_info == PLAYER_BULLET_INFO) {
    body_set_velocity(bullet, BULLET_VELOCITY);
  } else {
    body_set_velocity(bullet, vec_negate(BULLET_VELOCITY));
  }
  return bullet;
}

list_t *get_bodies_by_type(scene_t *scene, char *type) {
  int n = scene_bodies(scene);
  list_t *body_list = list_init(1, (free_func_t)body_free);
  for (int i = 0; i < n; i++) {
    body_t *body = scene_get_body(scene, i);
    if ((char *)(body_get_info(body)) == type) {
      list_add(body_list, body);
      // there's only one player, so this will break out of the loop to save
      // time
      if ((char *)(body_get_info(body)) == PLAYER_INFO) {
        break;
      }
    }
  }
  return body_list;
}

void key_handler_func(state_t *state, char key_pressed,
                      key_event_type_t event_type, double dt) {
  body_t *player = list_get(get_bodies_by_type(state->scene, PLAYER_INFO), 0);
  if (event_type == KEY_PRESSED) {
    switch (key_pressed) {
    case RIGHT_ARROW:
      body_set_velocity(player, RIGHT_VEL);
      break;
    case LEFT_ARROW:
      body_set_velocity(player, LEFT_VEL);
      break;
    case SPACE_BAR:
      PLAYER_TIME = PLAYER_TIME + time_since_last_tick();
      if (PLAYER_TIME > PLAYER_WAIT_TIME) {
        PLAYER_TIME = 0;
        scene_add_body(state->scene,
                       make_bullet(BULLET_MASS, RGB_GREEN,
                                   body_get_centroid(player), BULLET_RADIUS,
                                   PLAYER_BULLET_INFO));
      }
    default:
      break;
    }
  } else if (event_type == KEY_RELEASED) {
    body_set_velocity(player, VEC_ZERO);
  }
}

// check boundary
void check_player_boundary(state_t *state) {
  body_t *player = list_get(get_bodies_by_type(state->scene, PLAYER_INFO), 0);
  vector_t curr_centroid = body_get_centroid(player);
  if (X_SIZE - body_get_centroid(player).x < SEMIMAJOR) {
    body_set_centroid(player, (vector_t){X_SIZE - SEMIMAJOR, curr_centroid.y});
    body_set_velocity(player, VEC_ZERO);
  } else if (body_get_centroid(player).x < SEMIMAJOR) {
    body_set_centroid(player, (vector_t){SEMIMAJOR, curr_centroid.y});
    body_set_velocity(player, VEC_ZERO);
  }
}

// create initial alien arrangements
void make_aliens(state_t *state) {
  vector_t pos = {ALIEN_SPACING + ALIEN_RADIUS,
                  Y_SIZE - (ALIEN_SPACING + ALIEN_RADIUS)};
  for (int i = 0; i < ALIEN_COLS; i++) {
    scene_add_body(state->scene, make_alien(MASS, RGB_GRAY, pos, ALIEN_RADIUS));
    pos.y -= 2 * ALIEN_RADIUS;
    scene_add_body(state->scene, make_alien(MASS, RGB_GRAY, pos, ALIEN_RADIUS));
    pos.y -= 2 * ALIEN_RADIUS;
    scene_add_body(state->scene, make_alien(MASS, RGB_GRAY, pos, ALIEN_RADIUS));

    // reset to top of next column
    pos.x += 2 * ALIEN_RADIUS + ALIEN_SPACING;
    pos.y = Y_SIZE - (ALIEN_SPACING + ALIEN_RADIUS);
  }
}

void move_aliens(state_t *state) {
  list_t *aliens = get_bodies_by_type(state->scene, ALIEN_INFO);
  int n = list_size(aliens);
  for (int i = 0; i < n; i++) {
    body_t *alien = list_get(aliens, i);
    vector_t curr_centroid = body_get_centroid(alien);
    vector_t curr_velocity = body_get_velocity(alien);
    if (X_SIZE - body_get_centroid(alien).x < (ALIEN_RADIUS + ALIEN_SPACING)) {
      body_set_centroid(
          alien,
          (vector_t){X_SIZE - (ALIEN_RADIUS + ALIEN_SPACING),
                     curr_centroid.y - ALIEN_RADIUS_MULTIPLIER * ALIEN_RADIUS});
      body_set_velocity(alien, vec_negate(curr_velocity));
    } else if (body_get_centroid(alien).x < (ALIEN_RADIUS + ALIEN_SPACING)) {
      body_set_centroid(
          alien,
          (vector_t){(ALIEN_RADIUS + ALIEN_SPACING),
                     curr_centroid.y - ALIEN_RADIUS_MULTIPLIER * ALIEN_RADIUS});
      body_set_velocity(alien, vec_negate(curr_velocity));
    }
  }
}

bool check_collisions(scene_t *scene, char *body_info, char *bullet_info) {
  list_t *bodies = get_bodies_by_type(scene, body_info);
  list_t *bullets = get_bodies_by_type(scene, bullet_info);
  int n = list_size(bodies);
  int m = list_size(bullets);
  for (int i = 0; i < n; i++) {
    body_t *body = list_get(bodies, i);
    for (int j = 0; j < m; j++) {
      body_t *bullet = list_get(bullets, j);
      if ((find_collision(body_get_shape(body), body_get_shape(bullet)))
              .collided) {
        body_remove(body);
        body_remove(bullet);
        return true;
      }
    }
  }
  return false;
}

void remove_bodies_by_type(scene_t *scene, char *info) {
  list_t *bodies = get_bodies_by_type(scene, info);
  int n = list_size(bodies);
  for (int i = 0; i < n; i++) {
    body_t *body = list_get(bodies, i);
    if (body_get_centroid(body).y > Y_SIZE ||
        body_get_centroid(body).y < Y_ORIGIN) {
      body_remove(body);
    }
  }
}

void remove_bodies(scene_t *scene) {
  remove_bodies_by_type(scene, PLAYER_BULLET_INFO);
  remove_bodies_by_type(scene, ALIEN_BULLET_INFO);
  remove_bodies_by_type(scene, ALIEN_INFO);
}

void alien_shoot(scene_t *scene) {
  list_t *aliens = get_bodies_by_type(scene, ALIEN_INFO);
  if (list_size(aliens) == 0) {
    return;
  }
  ALIEN_TIME = ALIEN_TIME + time_since_last_tick();
  if (ALIEN_TIME > ALIEN_WAIT_TIME) {
    ALIEN_TIME = 0;
    body_t *alien = list_get(aliens, rand() % list_size(aliens));
    scene_add_body(scene,
                   make_bullet(BULLET_MASS, RGB_BLACK, body_get_centroid(alien),
                               BULLET_RADIUS, ALIEN_BULLET_INFO));
  }
}

void check_end_condition1(scene_t *scene) {
  if (list_size(get_bodies_by_type(scene, PLAYER_INFO)) == 0) {
    exit(0);
  }
}

void check_end_condition2(scene_t *scene) {
  if (list_size(get_bodies_by_type(scene, ALIEN_INFO)) == 0) {
    exit(0);
  }
}

void check_end_condition3(scene_t *scene) {
  list_t *aliens = get_bodies_by_type(scene, ALIEN_INFO);
  int n = list_size(aliens);
  for (int i = 0; i < n; i++) {
    body_t *alien = list_get(aliens, i);
    if (body_get_centroid(alien).y <= PLAYER_LEVEL * Y_SIZE) {
      exit(0);
    }
  }
}

void check_end_conditions(scene_t *scene) {
  // check if player got hit
  check_end_condition1(scene);
  // check if there are no more aliens
  check_end_condition2(scene);
  // check if aliens made past the player location
  check_end_condition3(scene);
}

state_t *emscripten_init() {
  srand(time(NULL));
  sdl_init(VEC_ZERO, (vector_t){X_SIZE, Y_SIZE});
  state_t *state = malloc(sizeof(state_t));
  state->scene = scene_init();
  // initialize player
  scene_add_body(state->scene,
                 make_player(MASS, RGB_GREEN,
                             (vector_t){X_SIZE / 2, PLAYER_LEVEL * Y_SIZE}));
  // initialize aliens
  make_aliens(state);
  sdl_on_key((key_handler_t)key_handler_func);
  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  scene_tick(state->scene, dt);
  // // check_end_conditions(state->scene);
  // // check_end_conditions(state->scene);

  check_end_conditions(state->scene);
  check_player_boundary(state);
  move_aliens(state);
  alien_shoot(state->scene);
  remove_bodies(state->scene);
  // printf("Number of bodies in scene: %d \n",
  // (int)scene_bodies(state->scene));
  // p rintf("Remaining number of aliens: %d \n",
  // (int)list_size(get_bodies_by_type(state->scene, ALIEN_INFO)));
  check_collisions(state->scene, ALIEN_INFO, PLAYER_BULLET_INFO);
  check_collisions(state->scene, PLAYER_INFO, ALIEN_BULLET_INFO);
  dt = time_since_last_tick();
  scene_tick(state->scene, dt);
  sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
