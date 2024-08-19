#include <voxy/scene/main_game/types/container.h>

#include <libcommon/core/log.h>

#include <stdlib.h>

struct container *container_create(struct item *items, size_t height, size_t width)
{
  struct container *container = malloc(sizeof *container);
  container->strong_count = 1;
  container->weak_count = 0;
  container->items = items;
  container->height = height;
  container->width = width;
  return container;
}

struct container *container_get_strong(struct container *container)
{
  container->strong_count += 1;
  return container;
}

struct container *container_get_weak(struct container *container)
{
  container->weak_count += 1;
  return container;
}

void container_put_strong(struct container **container)
{
  (*container)->strong_count -= 1;
  if((*container)->strong_count == 0 && (*container)->weak_count == 0)
    free(*container);

  *container = NULL;
}

void container_put_weak(struct container **container)
{
  (*container)->weak_count -= 1;
  if((*container)->strong_count == 0 && (*container)->weak_count == 0)
    free(*container);

  *container = NULL;
}
