#include "vector.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct vector_t;

vector_t vec_init(double x, double y) {
  vector_t v = {x, y};
  return v;
}

const vector_t VEC_ZERO = {0.0, 0.0};

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t v = {v1.x + v2.x, v1.y + v2.y};
  return v;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  vector_t v = {v1.x - v2.x, v1.y - v2.y};
  return v;
}

vector_t vec_negate(vector_t v) {
  vector_t neg = {(-1) * v.x, (-1) * v.y};
  return neg;
}

vector_t vec_multiply(double scalar, vector_t v) {
  vector_t mult = {scalar * v.x, scalar * v.y};
  return mult;
}

double vec_dot(vector_t v1, vector_t v2) {
  double dot = v1.x * v2.x + v1.y * v2.y;
  return dot;
}

double vec_cross(vector_t v1, vector_t v2) {
  double cross = v1.x * v2.y - v1.y * v2.x;
  return cross;
}

vector_t vec_norm(vector_t v) {
  double mag = sqrt(vec_dot(v, v));
  vector_t norm = vec_multiply((double)(1.0 / mag), v);
  return norm;
}

vector_t vec_rotate(vector_t v, double angle) {
  double x = v.x * cos(angle) - v.y * sin(angle);
  double y = v.x * sin(angle) + v.y * cos(angle);
  vector_t rotate = {x, y};
  return rotate;
}

void vec_free(vector_t *vector) { free(vector); }

vector_t unit_vector(vector_t v) {
  double magnitude = sqrt(vec_dot(v, v));
  return vec_multiply(1.0 / magnitude, v);
}