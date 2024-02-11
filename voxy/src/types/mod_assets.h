#ifndef TYPES_MOD_ASSETS_H
#define TYPES_MOD_ASSETS_H

#include <types/mod.h>

#include <graphics/gl.h>
#include <graphics/font_set.h>

struct mod_assets
{
  struct font_set             font_set;
  struct gl_array_texture_2d  block_array_texture;

  size_t                item_texture_count;
  struct gl_texture_2d *item_textures;
};

int mod_assets_load(struct mod_assets *mod_assets, struct mod *mod);
void mod_assets_unload(struct mod_assets *mod_assets);

#endif // TYPES_MOD_ASSETS_H
