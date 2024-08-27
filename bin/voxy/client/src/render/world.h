#ifndef RENDER_WORLD_H
#define RENDER_WORLD_H

#include "chunk/manager.h"
#include "blocks/blocks.h"

#include <libcommon/graphics/camera.h>

struct world_renderer
{
  struct blocks_renderer blocks;
};

int world_renderer_init(struct world_renderer *world_renderer);
void world_renderer_fini(struct world_renderer *world_renderer);

void world_renderer_update(struct world_renderer *world_renderer, struct chunk_manager *chunk_manager);
void world_renderer_render(struct world_renderer *world_renderer, const struct camera *camera);

#endif // RENDER_WORLD_H
