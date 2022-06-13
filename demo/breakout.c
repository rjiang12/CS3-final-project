#include "body.h"
#include "collision.h"
#include "forces.h"
#include "sdl_wrapper.h"
#include "time.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const double X_SIZE = 1110.0;
const double Y_SIZE = 1110.0;
const double X_ORIGIN = 0;
const double Y_ORIGIN = 0;
const rgb_color_t RGB_GRAY = {0.5, 0.5, 0.5};
const rgb_color_t RGB_BLACK = {0.0, 0.0, 0.0};
const rgb_color_t RGB_WHITE = {1, 1, 1};
const rgb_color_t RED = {1.0, 0.0, 0.0};
const rgb_color_t ORANGE = {1.0, 0.5, 0.0};
const rgb_color_t YELLOW = {1.0, 1.0, 0.0};
const rgb_color_t GREEN_1 = {0.5, 1.0, 0.0};
const rgb_color_t GREEN_2 = {0.0, 1.0, 0.0};
const rgb_color_t TEAL = {0.0, 1.0, 1.0};
const rgb_color_t BLUE = {0.0, 0.5, 1.0};
const rgb_color_t INDIGO = {0.0, 0.0, 1.0};
const rgb_color_t VIOLET = {1.0, 0.0, 1.0};
const rgb_color_t PINK = {1.0, 0.0, 0.5};
const double REC_HEIGHT = 25.0;
const double REC_WIDTH = 100.0;
const double MASS = 5.0;
const double SPACING = 10.0;
const int COLS = 10;
const vector_t LEFT_VEL = {-600, 0};
const vector_t RIGHT_VEL = {600, 0};
const double PLAYER_LEVEL = 0.05;
const int BALL_RADIUS = 10;
const int BALL_SIDES = 100;
const vector_t BALL_VELOCITY = {500, 500};
const int BRICK_THRESHOLD = 3;
const double BALL_INIT_Y = 100.0;
const double ELASTICITY = 1;
const char PLAYER_INFO_C = 'p';
const char BRICK_INFO_C = 'b';
const char BALL_INFO_C = 't';
const char WALL_INFO_C = 'w';
char *PLAYER_INFO = "p";
char *BRICK_INFO = "b";
char *BALL_INFO = "t";
char *WALL_INFO = "w";

typedef struct state {
  scene_t *scene;
  int bricks_struck;
  bool powerup_activated;
} state_t;

rgb_color_t color_wheel(int i) {
  switch (i) {
  case 0:
    return RED;
  case 1:
    return ORANGE;
  case 2:
    return YELLOW;
  case 3:
    return GREEN_1;
  case 4:
    return GREEN_2;
  case 5:
    return TEAL;
  case 6:
    return BLUE;
  case 7:
    return INDIGO;
  case 8:
    return VIOLET;
  case 9:
    return PINK;
  default:
    return RED;
  }
}

void add_vec_ptr(list_t *shape, double x, double y) {
  vector_t v = vec_init(x, y);
  vector_t *v_p = malloc(sizeof(v));
  *v_p = v;
  list_add(shape, v_p);
}

body_t *make_rectangle(double mass, rgb_color_t color, vector_t center,
                       char *info) {
  assert(center.x < X_SIZE && center.x > X_ORIGIN);
  assert(center.y < Y_SIZE && center.y > Y_ORIGIN);
  list_t *shape = list_init(4, free);

  add_vec_ptr(shape, REC_WIDTH / 2, -REC_HEIGHT / 2);
  add_vec_ptr(shape, REC_WIDTH / 2, REC_HEIGHT / 2);
  add_vec_ptr(shape, -REC_WIDTH / 2, REC_HEIGHT / 2);
  add_vec_ptr(shape, -REC_WIDTH / 2, -REC_HEIGHT / 2);

  body_t *rec_body =
      body_init_with_info(shape, mass, color, (void *)info, free);
  body_set_centroid(rec_body, center);
  return rec_body;
}

body_t *make_player(double mass, rgb_color_t color, vector_t center) {
  // makes rectangular player centered at the given center
  // (as a list_t) initialize the body using the list_t made above and the
  // arguments provided return the initialized body
  return make_rectangle(mass, color, center, PLAYER_INFO);
}

body_t *make_powerup_player(double mass, rgb_color_t color, vector_t center,
                            char *info) {
  assert(center.x < X_SIZE && center.x > X_ORIGIN);
  assert(center.y < Y_SIZE && center.y > Y_ORIGIN);
  list_t *shape = list_init(4, free);

  add_vec_ptr(shape, REC_WIDTH, -REC_HEIGHT / 2);
  add_vec_ptr(shape, REC_WIDTH, REC_HEIGHT / 2);
  add_vec_ptr(shape, -REC_WIDTH, REC_HEIGHT / 2);
  add_vec_ptr(shape, -REC_WIDTH, -REC_HEIGHT / 2);

  body_t *rec_body =
      body_init_with_info(shape, mass, color, (void *)info, free);
  body_set_centroid(rec_body, center);
  return rec_body;
}

body_t *make_ball(double mass, rgb_color_t color, vector_t center,
                  double size) {
  // makes a ball at given using the given attributes
  // initialize and return a body constituting that ball
  list_t *shape = list_init(BALL_SIDES, free);
  for (int i = 0; i < BALL_SIDES; i++) {
    vector_t vec = vec_init(size, 0);
    double angle = i * 2 * M_PI / BALL_SIDES;
    vec = vec_rotate(vec, angle);
    vector_t *vec_s = malloc(sizeof(vec));
    *vec_s = vec;
    list_add(shape, vec_s);
  }
  body_t *ball =
      body_init_with_info(shape, mass, color, (void *)BALL_INFO, free);
  body_set_centroid(ball, center);
  body_set_velocity(ball, BALL_VELOCITY);
  return ball;
}

void initialize_ball(state_t *state) {
  body_t *ball =
      make_ball(MASS, RED, (vector_t){X_SIZE / 2, BALL_INIT_Y}, BALL_RADIUS);
  scene_add_body(state->scene, ball);
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    switch (*((char *)body_get_info(body))) {
    case BRICK_INFO_C:
      create_physics_collision_remove_1(state->scene, 1, body, ball);
      break;
    case WALL_INFO_C:
      create_physics_collision(state->scene, 1, ball, body);
      break;
    case PLAYER_INFO_C:
      create_physics_collision(state->scene, 1, ball, body);
      break;
    case BALL_INFO_C:
      break;
    }
  }
}

list_t *get_bodies_by_type(scene_t *scene, char *type) {
  int n = scene_bodies(scene);
  list_t *body_list = list_init(1, (free_func_t)body_free);
  for (int i = 0; i < n; i++) {
    body_t *body = scene_get_body(scene, i);
    if (strcmp((char *)(body_get_info(body)), type) == 0) {
      list_add(body_list, body);
      if (strcmp((char *)(body_get_info(body)), PLAYER_INFO) == 0) {
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
    default:
      break;
    }
  } else if (event_type == KEY_RELEASED) {
    body_set_velocity(player, VEC_ZERO);
  }
}

// check boundary
void check_player_boundary(state_t *state, double width) {
  body_t *player = list_get(get_bodies_by_type(state->scene, PLAYER_INFO), 0);
  vector_t curr_centroid = body_get_centroid(player);
  if (X_SIZE - body_get_centroid(player).x < width) {
    body_set_centroid(player, (vector_t){X_SIZE - width, curr_centroid.y});
    body_set_velocity(player, VEC_ZERO);
  } else if (body_get_centroid(player).x < width) {
    body_set_centroid(player, (vector_t){width, curr_centroid.y});
    body_set_velocity(player, VEC_ZERO);
  }
}

// create brick arrangements
void make_bricks(state_t *state) {
  vector_t pos = {(REC_WIDTH / 2) + SPACING,
                  Y_SIZE - (SPACING + (REC_HEIGHT / 2))};
  for (int i = 0; i < COLS; i++) {
    scene_add_body(state->scene,
                   make_rectangle(INFINITY, color_wheel(i), pos, BRICK_INFO));
    pos.y -= (REC_HEIGHT + SPACING);
    scene_add_body(state->scene,
                   make_rectangle(INFINITY, color_wheel(i), pos, BRICK_INFO));
    pos.y -= (REC_HEIGHT + SPACING);
    scene_add_body(state->scene,
                   make_rectangle(INFINITY, color_wheel(i), pos, BRICK_INFO));
    // reset to top of next column
    pos.x += (REC_WIDTH + SPACING);
    pos.y = Y_SIZE - (SPACING + (REC_HEIGHT / 2));
  }
}

body_t *make_vertical_wall(double mass, vector_t center, char *info) {
  list_t *shape = list_init(4, free);

  add_vec_ptr(shape, REC_WIDTH / 2, -Y_SIZE / 2);
  add_vec_ptr(shape, REC_WIDTH / 2, Y_SIZE / 2);
  add_vec_ptr(shape, -REC_WIDTH / 2, Y_SIZE / 2);
  add_vec_ptr(shape, -REC_WIDTH / 2, -Y_SIZE / 2);

  body_t *rec_body =
      body_init_with_info(shape, mass, RGB_WHITE, (void *)info, free);
  body_set_centroid(rec_body, center);
  return rec_body;
}

body_t *make_horizontal_wall(double mass, vector_t center, char *info) {
  list_t *shape = list_init(4, free);

  add_vec_ptr(shape, X_SIZE / 2, -REC_HEIGHT / 2);
  add_vec_ptr(shape, X_SIZE / 2, REC_HEIGHT / 2);
  add_vec_ptr(shape, -X_SIZE / 2, REC_HEIGHT / 2);
  add_vec_ptr(shape, -X_SIZE / 2, -REC_HEIGHT / 2);

  body_t *rec_body =
      body_init_with_info(shape, mass, RGB_WHITE, (void *)info, free);
  body_set_centroid(rec_body, center);
  return rec_body;
}

void make_walls(state_t *state) {
  scene_add_body(
      state->scene,
      make_horizontal_wall(INFINITY,
                           (vector_t){(X_SIZE / 2), Y_SIZE + (REC_HEIGHT / 2)},
                           WALL_INFO));
  scene_add_body(state->scene,
                 make_vertical_wall(INFINITY,
                                    (vector_t){-(REC_WIDTH / 2), Y_SIZE / 2},
                                    WALL_INFO));
  scene_add_body(state->scene,
                 make_vertical_wall(
                     INFINITY, (vector_t){X_SIZE + (REC_WIDTH / 2), Y_SIZE / 2},
                     WALL_INFO));
}

bool check_collisions(scene_t *scene, char *body_info, char *ball_info) {
  list_t *bodies = get_bodies_by_type(scene, body_info);
  list_t *balls = get_bodies_by_type(scene, ball_info);
  int n = list_size(bodies);
  int m = list_size(balls);
  for (int i = 0; i < n; i++) {
    body_t *body = list_get(bodies, i);
    for (int j = 0; j < m; j++) {
      body_t *ball = list_get(balls, j);
      if ((find_collision(body_get_shape(body), body_get_shape(ball))
               .collided)) {
        create_physics_collision(scene, ELASTICITY, ball, body);
        if (strcmp(body_get_info(body), BRICK_INFO) == 0) {
          body_remove(body);
        }
        free(bodies);
        free(balls);
        return true;
      }
    }
  }
  free(bodies);
  free(balls);
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

void reset(state_t *state) {
  scene_free(state->scene);
  state->scene = scene_init();
  state->bricks_struck = 0;
  state->powerup_activated = false;
  // initialize player
  scene_add_body(state->scene,
                 make_player(INFINITY, RED,
                             (vector_t){X_SIZE / 2, PLAYER_LEVEL * Y_SIZE}));
  // initialize bricks
  make_bricks(state);
  // initialize walls
  make_walls(state);
  initialize_ball(state);
}

void check_end_conditions(state_t *state) {
  body_t *ball = list_get(get_bodies_by_type(state->scene, BALL_INFO), 0);
  if (body_get_centroid(ball).y <= PLAYER_LEVEL * Y_SIZE) {
    reset(state);
  }
  if (list_size(get_bodies_by_type(state->scene, BRICK_INFO)) == 0) {
    exit(0);
  }
}

state_t *emscripten_init() {
  srand(time(NULL));
  sdl_init(VEC_ZERO, (vector_t){X_SIZE, Y_SIZE});
  state_t *state = malloc(sizeof(state_t));
  state->scene = scene_init();
  state->bricks_struck = 0;
  state->powerup_activated = false;
  // initialize player
  scene_add_body(state->scene,
                 make_player(INFINITY, RED,
                             (vector_t){X_SIZE / 2, PLAYER_LEVEL * Y_SIZE}));
  // initialize bricks
  make_bricks(state);
  // initialize walls
  make_walls(state);
  // initialize ball
  initialize_ball(state);
  sdl_on_key((key_handler_t)key_handler_func);
  return state;
}

void check_powerup_status(size_t num_bricks_before, state_t *state) {
  list_t *list_after = get_bodies_by_type(state->scene, BRICK_INFO);
  size_t num_bricks_after = list_size(list_after);
  if ((int)num_bricks_after < (int)num_bricks_before) {
    state->bricks_struck =
        state->bricks_struck + ((int)num_bricks_before - (int)num_bricks_after);
    if (state->bricks_struck >= BRICK_THRESHOLD &&
        !(state->powerup_activated)) {
      printf("POWER-UP ACTIVATED! \n");
      state->powerup_activated = true;
      body_t *old_player =
          list_get(get_bodies_by_type(state->scene, PLAYER_INFO), 0);
      vector_t centroid = body_get_centroid(old_player);
      body_remove(old_player);
      body_t *new_player = make_powerup_player(
          INFINITY, RED, (vector_t){centroid.x, PLAYER_LEVEL * Y_SIZE},
          PLAYER_INFO);
      body_t *ball = list_get(get_bodies_by_type(state->scene, BALL_INFO), 0);
      create_physics_collision(state->scene, ELASTICITY, new_player, ball);
      scene_add_body(state->scene, new_player);
      state->bricks_struck = 0;
    } else if (state->bricks_struck >= BRICK_THRESHOLD &&
               state->powerup_activated) {
      printf("POWER-UP DEACTIVATED. \n");
      state->powerup_activated = false;
      body_t *old_player =
          list_get(get_bodies_by_type(state->scene, PLAYER_INFO), 0);
      vector_t centroid = body_get_centroid(old_player);
      body_remove(old_player);
      body_t *new_player = make_player(
          INFINITY, RED, (vector_t){centroid.x, PLAYER_LEVEL * Y_SIZE});
      body_t *ball = list_get(get_bodies_by_type(state->scene, BALL_INFO), 0);
      create_physics_collision(state->scene, ELASTICITY, new_player, ball);
      scene_add_body(state->scene, new_player);
      state->bricks_struck = 0;
    }
  }
  free(list_after);
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  list_t *list_before = get_bodies_by_type(state->scene, BRICK_INFO);
  size_t num_bricks_before = list_size(list_before);
  scene_tick(state->scene, dt);
  check_powerup_status(num_bricks_before, state);
  free(list_before);
  check_end_conditions(state);
  if (state->powerup_activated) {
    check_player_boundary(state, REC_WIDTH);
  } else {
    check_player_boundary(state, REC_WIDTH / 2);
  }
  sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
