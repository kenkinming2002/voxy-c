#ifndef RENDER_BLOCKS_RENDER_INFO_H
#define RENDER_BLOCKS_RENDER_INFO_H

#include "chunk/chunk.h"
#include "mesh.h"

#include <libcommon/graphics/camera.h>

/// Render info for blocks in a chunk i.e. handles to meshes of blocks in a
/// chunk and a boolean for doing some form of frustum culling.
///
/// Currently, we only have two meshes - opaque and transparent -  so that we
/// can draw trasparent blocks after opaque blocks.
struct blocks_render_info
{
  size_t                     hash;
  struct blocks_render_info *next;

  ivec3_t position;

  struct blocks_mesh opaque_mesh;
  struct blocks_mesh transparent_mesh;

  bool culled;
};

/// Create/destroy blocks render info.
struct blocks_render_info *blocks_render_info_create(void);
void blocks_render_info_destroy(struct blocks_render_info *blocks_render_info);

/// Update blocks render info for chunk.
///
/// This need to be called whenever any blocks in a chunk have changed which
/// should be indicated by some form of a *dirty* flag on the chunk.
///
/// FIXME: Add back the digger argument.
void blocks_render_info_update(struct blocks_render_info *blocks_render_info, const struct chunk *chunk);

/// Determine if a chunk can be culled when rendered with the given camera.
///
/// The result would be stored in the cull variable.
void blocks_render_info_update_cull(struct blocks_render_info *blocks_render_info, const struct camera *camera);

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX blocks_render_info
#define SC_HASH_TABLE_NODE_TYPE struct blocks_render_info
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_INTERFACE

#endif // RENDER_BLOCKS_RENDER_INFO_H
