#include "world.h"

int world_renderer_init(struct world_renderer *world_renderer)
{
  return blocks_renderer_init(&world_renderer->blocks);
}

void world_renderer_fini(struct world_renderer *world_renderer)
{
  blocks_renderer_fini(&world_renderer->blocks);
}

void world_renderer_update(struct world_renderer *world_renderer, struct chunk_manager *chunk_manager)
{
  blocks_renderer_update(&world_renderer->blocks, chunk_manager);
}

void world_renderer_render(struct world_renderer *world_renderer, const struct camera *camera)
{
  blocks_renderer_render(&world_renderer->blocks, camera);
}
