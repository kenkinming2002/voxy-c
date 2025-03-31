#ifndef RENDER_BLOCK_RENDER_INFO_H
#define RENDER_BLOCK_RENDER_INFO_H

#include "chunk/block/group.h"
#include "mesh.h"

#include <libgfx/camera.h>

struct block_renderer;
struct voxy_block_registry;

/// Render info for blocks in a block group i.e. handles to meshes of blocks in a
/// block group and a boolean for doing some form of frustum culling.
///
/// Currently, we only have two meshes - opaque and transparent -  so that we
/// can draw trasparent blocks after opaque blocks.
struct block_render_info
{
  struct block_mesh opaque_mesh;
  struct block_mesh transparent_mesh;
  bool culled;
};

/// Create/destroy blocks render info.
struct block_render_info block_render_info_create(void);
void block_render_info_destroy(struct block_render_info block_render_info);

/// Update blocks render info for block group.
///
/// This need to be called whenever any blocks in a block group have changed which
/// should be indicated by some form of a *dirty* flag on the block group.
///
/// FIXME: Add back the digger argument.
void block_render_info_update(struct block_render_info *block_render_info, struct voxy_block_registry *block_registry, struct block_renderer *block_renderer, ivec3_t position, const struct block_group *block_group);

/// Determine if a block group can be culled when rendered with the given camera.
///
/// The result would be stored in the cull variable.
void block_render_info_update_cull(ivec3_t position, struct block_render_info *block_render_info, const struct camera *camera);

#endif // RENDER_BLOCK_RENDER_INFO_H
