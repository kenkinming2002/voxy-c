#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H

#include "registry/block.h"
#include "chunk/manager.h"

#include <libcore/dynamic_array.h>

#include <stdint.h>

/// References:
///  - https://www.seedofandromeda.com/blogs/29-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-1
///  - https://www.seedofandromeda.com/blogs/30-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-2
/// The link is dead but the article could be still be accessed via wayback machine.

struct light_destruction_update
{
  struct voxy_chunk *chunk;
  uint8_t x;
  uint8_t y;
  uint8_t z;
  uint8_t old_light_level;
};

struct light_creation_update
{
  struct voxy_chunk *chunk;
  uint8_t x;
  uint8_t y;
  uint8_t z;
};

DYNAMIC_ARRAY_DEFINE(light_creation_updates, struct light_creation_update);
DYNAMIC_ARRAY_DEFINE(light_destruction_updates, struct light_destruction_update);

/// Light manager.
///
/// Yes, I do realize that I am naming everything manager, but I do not really
/// want to spend time thinking of a better name.
///
/// Ideally, we would simply have a free-standing function light_update(), but
/// we do not live in an ideal world, and we have hidden internal state
/// everywhere.
struct voxy_light_manager
{
  struct light_creation_updates light_creation_updates;
  struct light_destruction_updates light_destruction_updates;
};

void voxy_light_manager_init(struct voxy_light_manager *light_manager);
void voxy_light_manager_fini(struct voxy_light_manager *light_manager);

void voxy_light_manager_enqueue_destruction_update_at(struct voxy_light_manager *light_manager, struct voxy_chunk *chunk, ivec3_t position, uint8_t light_level);
void voxy_light_manager_enqueue_creation_update_at(struct voxy_light_manager *light_manager, struct voxy_chunk *chunk, ivec3_t position);

void voxy_light_manager_enqueue_destruction_update(struct voxy_light_manager *light_manager, struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t light_level);
void voxy_light_manager_enqueue_creation_update(struct voxy_light_manager *light_manager, struct voxy_chunk_manager *chunk_manager, ivec3_t position);

void voxy_light_manager_update(struct voxy_light_manager *light_manager, struct voxy_block_registry *block_registry);

#endif // LIGHT_MANAGER_H
