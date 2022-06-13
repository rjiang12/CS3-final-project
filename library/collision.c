#include "collision.h"
#include "list.h"
#include "vector.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

double *min_max(list_t *shape, vector_t axis) {
  double *minmax = malloc(2 * sizeof(double));
  double min = INFINITY;
  double max = -INFINITY;
  for (size_t j = 0; j < list_size(shape); j++) {
    double point = vec_dot(*(vector_t *)list_get(shape, j), axis);
    if (point < min) {
      min = point;
    }
    if (point > max) {
      max = point;
    }
  }
  minmax[0] = min;
  minmax[1] = max;
  return minmax;
}

collision_info_t find_collision(list_t *shape1, list_t *shape2) {
  collision_info_t collision = {.collided = false};
  double min = INFINITY;
  for (size_t i = 0; i < list_size(shape1); i++) {
    vector_t edge = vec_subtract(
        *(vector_t *)list_get(shape1, i),
        *(vector_t *)list_get(shape1, (i + 1) % list_size(shape1)));
    vector_t axis = vec_norm((vector_t){edge.y, -edge.x});
    double *min_and_max1 = min_max(shape1, axis);
    double min1 = min_and_max1[0];
    double max1 = min_and_max1[1];
    double *min_and_max2 = min_max(shape2, axis);
    double min2 = min_and_max2[0];
    double max2 = min_and_max2[1];
    free(min_and_max1);
    free(min_and_max2);
    if ((min1 > max2 || min2 > max1)) {
      return collision;
    }
    double overlap = fmin(max2 - min1, max1 - min2);

    if (overlap < min) {
      collision.axis = axis;
      min = overlap;
    }
  }
  for (size_t i = 0; i < list_size(shape2); i++) {
    vector_t edge = vec_subtract(
        *(vector_t *)list_get(shape2, i),
        *(vector_t *)list_get(shape2, (i + 1) % list_size(shape2)));
    vector_t axis = vec_norm((vector_t){edge.y, -edge.x});
    double *min_and_max1 = min_max(shape1, axis);
    double min1 = min_and_max1[0];
    double max1 = min_and_max1[1];
    double *min_and_max2 = min_max(shape2, axis);
    double min2 = min_and_max2[0];
    double max2 = min_and_max2[1];
    free(min_and_max1);
    free(min_and_max2);
    if ((min1 > max2 || min2 > max1)) {
      return collision;
    }
    double overlap = fmin(max2 - min1, max1 - min2);
    if (overlap < min) {
      collision.axis = axis;
      min = overlap;
    }
  }
  collision.collided = true;
  return collision;
}
