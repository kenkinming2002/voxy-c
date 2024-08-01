#ifndef VOXY_DYNAMIC_ARRAY_H
#define VOXY_DYNAMIC_ARRAY_H

#include <stdlib.h>

#define DYNAMIC_ARRAY_DEFINE(name, type) \
  struct name \
  { \
    type *items; \
    size_t item_count; \
    size_t item_capacity; \
  }

#define DYNAMIC_ARRAY_DECLARE(array, type) \
  struct \
  { \
    type *items; \
    size_t item_count; \
    size_t item_capacity; \
  } array = {0}

#define DYNAMIC_ARRAY_INIT(array) \
  do \
  { \
    (array).items = NULL; \
    (array).item_count = 0; \
    (array).item_capacity = 0; \
  } \
  while(0)

#define DYNAMIC_ARRAY_CLEAR(array) \
  do \
  { \
    free((array).items); \
    (array).items = NULL; \
    (array).item_count = 0; \
    (array).item_capacity = 0; \
  } \
  while(0)

#define DYNAMIC_ARRAY_RESERVE(array, capacity) \
  do \
  { \
    if((array).item_capacity < (capacity)) \
    { \
      (array).item_capacity = (capacity); \
      (array).items = realloc((array).items, (array).item_capacity * sizeof *(array).items); \
    } \
  } \
  while(0)

#define DYNAMIC_ARRAY_APPEND(array, value) \
  do \
  { \
    if((array).item_capacity == (array).item_count) \
    { \
      (array).item_capacity = (array).item_capacity != 0 ? (array).item_capacity * 2 : 1; \
      (array).items         = realloc((array).items, (array).item_capacity * sizeof *(array).items); \
    } \
    (array).items[(array).item_count++] = value; \
  } \
  while(0)

#define DYNAMIC_ARRAY_APPEND_MANY(array, new_items, new_item_count) \
  do \
  { \
    while((array).item_capacity < (array).item_count + (new_item_count)) \
      (array).item_capacity = (array).item_capacity != 0 ? (array).item_capacity * 2 : 1; \
    (array).items = realloc((array).items, (array).item_capacity * sizeof *(array).items); \
    for(size_t i=0; i<(new_item_count); ++i) \
      (array).items[(array).item_count++] = (new_items)[i]; \
  } \
  while(0)

#endif // VOXY_DYNAMIC_ARRAY_H
