#ifndef RENDER_BLOCKS_BLOCKS_H
#define RENDER_BLOCKS_BLOCKS_H

#include "chunk/manager.h"
#include "block/registry.h"
#include "render_info.h"

#include <libcommon/graphics/gl.h>

struct blocks_renderer
{
  struct gl_program program;

  struct gl_array_texture_2d texture;
  uint32_t (*texture_indices)[DIRECTION_COUNT];

  struct blocks_render_info_hash_table render_infos;
};

int blocks_renderer_init(struct blocks_renderer *blocks_renderer, const struct block_registry *block_registry);
void blocks_renderer_fini(struct blocks_renderer *blocks_renderer);

void blocks_renderer_update(struct blocks_renderer *blocks_renderer, struct block_registry *block_registry, struct chunk_manager *chunk_manager);
void blocks_renderer_render(struct blocks_renderer *blocks_renderer, const struct camera *camera);

#endif // RENDER_BLOCKS_BLOCKS_H
