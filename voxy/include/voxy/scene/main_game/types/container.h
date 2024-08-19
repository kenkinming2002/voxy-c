#ifndef VOXY_SCENE_MAIN_GAME_TYPES_CONTAINER_H
#define VOXY_SCENE_MAIN_GAME_TYPES_CONTAINER_H

#include "item.h"

struct container
{
  size_t strong_count;
  size_t weak_count;

  struct item *items;
  size_t height;
  size_t width;
};

struct container *container_create(struct item *items, size_t height, size_t width);

struct container *container_get_strong(struct container *container);
struct container *container_get_weak(struct container *container);

void container_put_strong(struct container **container);
void container_put_weak(struct container **container);

#endif // VOXY_SCENE_MAIN_GAME_TYPES_CONTAINER_H
