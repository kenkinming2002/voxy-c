#include "world.h"

int world_renderer_init(struct world_renderer *world_renderer, struct block_registry *block_registry)
{
  return block_renderer_init(&world_renderer->block, block_registry);
}

void world_renderer_fini(struct world_renderer *world_renderer)
{
  block_renderer_fini(&world_renderer->block);
}

void world_renderer_update(struct world_renderer *world_renderer, struct block_registry *block_registry, struct chunk_manager *chunk_manager)
{
  block_renderer_update(&world_renderer->block, block_registry, chunk_manager);
}

void world_renderer_render(struct world_renderer *world_renderer, const struct camera *camera)
{
  block_renderer_render(&world_renderer->block, camera);
}
