#ifndef VOXY_MAIN_GAME_MOD_ASSETS_H
#define VOXY_MAIN_GAME_MOD_ASSETS_H

#include <stdint.h>

struct font_set;
struct gl_array_texture_2d;
struct gl_texture_2d;

struct font_set            *mod_assets_font_set_get(void);
struct gl_array_texture_2d *mod_assets_block_array_texture_get(void);
struct gl_texture_2d       *mod_assets_item_texture_get(uint8_t item_id);

#endif // VOXY_MAIN_GAME_MOD_ASSETS_H
