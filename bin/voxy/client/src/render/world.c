#include "world.h"
#include "render/block/block.h"
#include "render/entity/entity.h"

int world_renderer_init(struct world_renderer *world_renderer)
{
  block_renderer_init();
  return 0;
}

void world_renderer_fini(struct world_renderer *world_renderer)
{
}

void world_renderer_update(struct world_renderer *world_renderer)
{
  block_renderer_update();
}

void world_renderer_render(struct world_renderer *world_renderer)
{
  block_renderer_render();
  entity_renderer_render();
}
