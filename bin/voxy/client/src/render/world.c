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

void render_world(void)
{
  render_block();
  render_entity();
}
