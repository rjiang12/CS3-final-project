#include "list.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int RESIZING_FACTOR = 2;

struct list {
  void **s;
  size_t size;
  size_t capacity;
  free_func_t freer;
};

list_t *list_init(size_t initial_size, free_func_t free_func) {
  list_t *l = malloc(sizeof(list_t));
  l->s = malloc(initial_size * sizeof(void *));
  l->size = 0;
  l->capacity = initial_size;
  l->freer = free_func;
  return l;
}

void list_free(list_t *l) {
  // individually free() each pointer in the array
  size_t list_size = l->size;
  for (size_t i = 0; i < list_size; i++) {
    l->freer(list_get(l, i));
  }
  // free() the array and struct itself
  free(l->s);
  free(l);
}

size_t list_size(list_t *l) { return l->size; }

void *list_get(list_t *l, size_t index) {
  assert(index < list_size(l));
  void *v = l->s[index];
  return v;
}

void list_set(list_t *l, size_t index, void *value) {
  assert(value != NULL);
  assert(index < list_size(l));
  l->s[index] = value;
  if (index >= l->size) {
    l->size++;
  }
  assert(l->s[index] != NULL);
}

void list_resize(list_t *l) {
  size_t new_cap = l->capacity * RESIZING_FACTOR;
  l->s = realloc(l->s, new_cap * sizeof(void *));
  l->capacity = new_cap;
}

void list_add(list_t *l, void *value) {
  if (l->size >= l->capacity) {
    list_resize(l);
  }
  assert(l->size < l->capacity);
  l->s[l->size] = value;
  l->size++;
}

void *list_remove(list_t *l, size_t index) {
  assert(l->size > index);
  void *removed = list_get(l, index);
  for (size_t i = index; i < l->size - 1; i++) {
    l->s[i] = l->s[i + 1];
  }
  l->s[l->size - 1] = NULL;
  l->size--;
  return removed;
}

void list_clear(list_t *l) {
  while (list_size(l) > 0) {
    l->freer(list_remove(l, 0));
  }
}
