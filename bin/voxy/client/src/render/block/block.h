#ifndef RENDER_BLOCK_BLOCK_H
#define RENDER_BLOCK_BLOCK_H

#include <voxy/client/registry/block.h>

#include <libgfx/gl.h>

void block_renderer_init(void);
void block_renderer_update(void);

void render_block(void);

uint32_t block_renderer_get_texture_index(voxy_block_id_t id, direction_t direction);

#endif // RENDER_BLOCK_BLOCK_H
