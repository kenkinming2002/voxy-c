#ifndef RENDER_WORLD_H
#define RENDER_WORLD_H

#include "block/block.h"

#include "chunk/entity/manager.h"

#include <libgfx/camera.h>

struct world_renderer
{
  struct block_renderer block;
};

int world_renderer_init(struct world_renderer *world_renderer);
void world_renderer_fini(struct world_renderer *world_renderer);

void world_renderer_update(struct world_renderer *world_renderer);
void world_renderer_render(struct world_renderer *world_renderer, struct entity_manager *entity_manager);

#endif // RENDER_WORLD_H
