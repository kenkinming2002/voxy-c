#ifndef RENDER_WORLD_H
#define RENDER_WORLD_H

#include "block/block.h"

#include "chunk/manager.h"
#include "block/registry.h"

#include <libcommon/graphics/camera.h>

struct world_renderer
{
  struct block_renderer block;
};

int world_renderer_init(struct world_renderer *world_renderer, struct block_registry *block_registry);
void world_renderer_fini(struct world_renderer *world_renderer);

void world_renderer_update(struct world_renderer *world_renderer, struct block_registry *block_registry, struct chunk_manager *chunk_manager);
void world_renderer_render(struct world_renderer *world_renderer, const struct camera *camera);

#endif // RENDER_WORLD_H
