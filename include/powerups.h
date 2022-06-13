#ifndef __POWERUPS_H__
#define __POWERUPS_H__

#include "list.h"
#include "vector.h"
#include "body.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char DOUBLE_VELOCITY = 'v';
const char DOUBLE_ACCELERATION = 'a';
const char HALF_ACCELERATION = 'b';
const char FREEZE_ENEMY = 'f';
const char DOUBLE_GOAL = 'g';

void double_velocity(body_t *puck) {
  vector_t curr_velocity = body_get_velocity(puck); 
  body_set_velocity(puck, vec_multiply(2, curr_velocity));
}

void double_accel(body_t *last_touch) {
  body_set_velocity(last_touch, vec_multiply(2.0, body_get_velocity(last_touch)));   
}

void half_accel(body_t *other_player) {
  time_t start = time(NULL);
  time_t now = time(NULL); 
  while((now - start) < 5) {
    body_set_velocity(other_player, vec_multiply(0.5, body_get_velocity(other_player))); 
  }
}

void freeze_enemy(body_t *other_player) {
  time_t start = time(NULL);
  time_t now = time(NULL); 
  while((now - start) < 5) {
    body_set_velocity(other_player, (vector_t) {0, 0});
  }
}

void double_goal(state_t *state) {
  state->ppg = PPG_POWERUP; 
}

void powerup_collision(state_t *state, body_t *puck, body_t *powerup) {
  switch(*((char *) body_get_info(powerup))) {
    case DOUBLE_VELOCITY:
      double_velocity(puck); 
    case DOUBLE_ACCELERATION:
      double_accel(state->last_touch); 
    case HALF_ACCELERATION:
      double_accel(state->other_player);
    case FREEZE_ENEMY:
      freeze_enemy(state->other_player);
    case DOUBLE_GOAL:
      double_goal(state); 
    default:
      return; 
  }
}

#endif // #ifndef __POWERUPS_H__