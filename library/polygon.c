#include "polygon.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double polygon_area(list_t *polygon) {
  double area = 0;
  // implementation of shoelace formula
  for (int i = 0; i < (int)list_size(polygon); i++) {
    area += vec_cross(((vector_t *)list_get(polygon, (size_t)i))[0],
                      ((vector_t *)list_get(
                          polygon, (size_t)((i + 1) % list_size(polygon))))[0]);
  }
  area /= 2;
  return area;
}

vector_t polygon_centroid(list_t *polygon) {
  double centroid_x = 0;
  double centroid_y = 0;
  // implementation of provided formula
  for (int i = 0; i < list_size(polygon); i++) {
    double x_i = ((vector_t *)list_get(polygon, (size_t)i))->x;
    double y_i = ((vector_t *)list_get(polygon, (size_t)i))->y;
    double x_i1;
    double y_i1;
    x_i1 = ((vector_t *)list_get(polygon, (size_t)(i + 1) % list_size(polygon)))
               ->x;
    y_i1 = ((vector_t *)list_get(polygon, (size_t)(i + 1) % list_size(polygon)))
               ->y;
    centroid_x += (x_i + x_i1) * (x_i * y_i1 - x_i1 * y_i);
    centroid_y += (y_i + y_i1) * (x_i * y_i1 - x_i1 * y_i);
  }
  centroid_x /= 6 * polygon_area(polygon);
  centroid_y /= 6 * polygon_area(polygon);
  vector_t centroid = {centroid_x, centroid_y};
  return centroid;
}

void polygon_translate(list_t *polygon, vector_t translation) {
  // translate each individual vector using vec_add()
  for (int i = 0; i < (int)list_size(polygon); i++) {
    vector_t *pointer = list_get(polygon, i);
    vector_t value = pointer[0];
    vector_t new = vec_add(value, translation);
    *pointer = new;
  }
  return;
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
  // translate from rotation point to origin
  vector_t to_origin = vec_negate(point);
  polygon_translate(polygon, to_origin);
  // rotate each individual vector in polygon
  for (int i = 0; i < (int)list_size(polygon); i++) {
    vector_t *pointer = list_get(polygon, i);
    vector_t value = pointer[0];
    vector_t rotated = vec_rotate(value, angle);
    *pointer = rotated;
  }
  // translate from origin back to rotation point
  polygon_translate(polygon, point);
  return;
}