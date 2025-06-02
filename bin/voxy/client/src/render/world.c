#include "world.h"

#include "render/block/block.h"
#include "render/entity/entity.h"

void world_renderer_init(void)
{
  block_renderer_init();
}

void world_renderer_update(void)
{
  block_renderer_update();
}

void world_renderer_render(void)
{
  block_renderer_render();
  entity_renderer_render();
}
