#ifndef VOXY_SCENE_MAIN_GAME_RENDER_BLOCKS_RENDER_INFO_H
#define VOXY_SCENE_MAIN_GAME_RENDER_BLOCKS_RENDER_INFO_H

#include <voxy/scene/main_game/types/chunk.h>
#include <voxy/scene/main_game/states/digger.h>

#include <libcommon/graphics/camera.h>
#include <libcommon/math/vector.h>

#include "blocks_mesh.h"

struct blocks_render_info
{
  size_t hash;
  struct blocks_render_info *next;

  ivec3_t position;

  bool culled;
  struct blocks_mesh opaque_mesh;
  struct blocks_mesh transparent_mesh;
};

struct blocks_render_info *blocks_render_info_create(void);
void blocks_render_info_destroy(struct blocks_render_info *blocks_render_info);

void blocks_render_info_update(struct blocks_render_info *blocks_render_info, const struct chunk *chunk, const struct digger *digger);
void blocks_render_info_cull(struct blocks_render_info *blocks_render_info, const struct camera *camera);

void blocks_render_info_render_begin(const struct camera *camera);
void blocks_render_info_render_opaque(const struct blocks_render_info *blocks_render_info);
void blocks_render_info_render_transparent(const struct blocks_render_info *blocks_render_info);

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX blocks_render_info
#define SC_HASH_TABLE_NODE_TYPE struct blocks_render_info
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_INTERFACE

ivec3_t blocks_render_info_key(struct blocks_render_info *blocks_render_info);
size_t blocks_render_info_hash(ivec3_t position);
int blocks_render_info_compare(ivec3_t position1, ivec3_t position2);
void blocks_render_info_dispose(struct blocks_render_info *blocks_render_info);

#endif // VOXY_SCENE_MAIN_GAME_RENDER_BLOCKS_RENDER_INFO_H
