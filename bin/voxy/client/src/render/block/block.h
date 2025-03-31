#ifndef RENDER_BLOCK_BLOCK_H
#define RENDER_BLOCK_BLOCK_H

#include "registry/block.h"
#include "chunk/block/manager.h"
#include "camera/manager.h"
#include "render_info.h"

#include <libgfx/gl.h>

struct block_render_info_node
{
  ivec3_t key;
  struct block_render_info value;
};

struct block_renderer
{
  struct gl_program program;

  struct gl_array_texture_2d texture;
  uint32_t (*texture_indices)[DIRECTION_COUNT];

  struct block_render_info_node *render_info_nodes;
};

int block_renderer_init(struct block_renderer *block_renderer, const struct voxy_block_registry *block_registry);
void block_renderer_fini(struct block_renderer *block_renderer);

void block_renderer_update(struct block_renderer *block_renderer, struct voxy_block_registry *block_registry, struct block_manager *block_manager, struct camera_manager *camera_manager);
void block_renderer_render(struct block_renderer *block_renderer, struct camera_manager *camera_manager);

#endif // RENDER_BLOCK_BLOCK_H
