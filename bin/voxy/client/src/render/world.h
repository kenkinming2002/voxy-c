#ifndef RENDER_WORLD_H
#define RENDER_WORLD_H

#include "block/block.h"
#include "entity/entity.h"

#include "registry/block.h"
#include "registry/entity.h"

#include "chunk/manager.h"
#include "entity/manager.h"

#include "camera/manager.h"

#include <libcommon/graphics/camera.h>

struct world_renderer
{
  struct block_renderer block;
};

int world_renderer_init(struct world_renderer *world_renderer, struct voxy_block_registry *block_registry);
void world_renderer_fini(struct world_renderer *world_renderer);

void world_renderer_update(struct world_renderer *world_renderer, struct voxy_block_registry *block_registry, struct chunk_manager *chunk_manager, struct camera_manager *camera_manager);
void world_renderer_render(struct world_renderer *world_renderer, struct voxy_entity_registry *entity_registry, struct entity_manager *entity_manager, struct camera_manager *camera_manager);

#endif // RENDER_WORLD_H
