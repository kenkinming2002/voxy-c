#ifndef VOXY_MAIN_GAME_RENDER_ASSETS_H
#define VOXY_MAIN_GAME_RENDER_ASSETS_H

#include <voxy/main_game/types/registry.h>

#include <stdint.h>

struct gl_texture_2d assets_get_item_texture(item_id_t item_id);

struct gl_array_texture_2d assets_get_block_texture_array(void);
uint32_t                   assets_get_block_texture_array_index(block_id_t block_id, enum block_face block_face);

#endif // VOXY_MAIN_GAME_RENDER_ASSETS_H
