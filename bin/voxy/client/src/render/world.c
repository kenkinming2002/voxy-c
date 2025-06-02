#include "world.h"
#include "render/entity/entity.h"

int world_renderer_init(struct world_renderer *world_renderer)
{
  return block_renderer_init(&world_renderer->block);
}

void world_renderer_fini(struct world_renderer *world_renderer)
{
  block_renderer_fini(&world_renderer->block);
}

void world_renderer_update(struct world_renderer *world_renderer)
{
  block_renderer_update(&world_renderer->block);
}

void world_renderer_render(struct world_renderer *world_renderer, struct entity_manager *entity_manager)
{
  block_renderer_render(&world_renderer->block);
  entity_renderer_render(entity_manager);
}
