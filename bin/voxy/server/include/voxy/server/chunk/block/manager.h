#ifndef VOXY_SERVER_CHUNK_BLOCK_MANAGER_H
#define VOXY_SERVER_CHUNK_BLOCK_MANAGER_H

#include <voxy/server/export.h>
#include <voxy/server/registry/block.h>

#include <libmath/vector.h>

#include <stdbool.h>

/// FIXME: Figure out a better API so that we no longer have to perform repeated
///        chunk lookup each time we call a getter or setter.

/// Block id/light level getters.
///
/// In case the block dees not exist (for example if the chunk in which the
/// block resides in has not yet been loaded/generated), value in def is
/// returned.
VOXY_SERVER_EXPORT voxy_block_id_t voxy_get_block_id(ivec3_t position, voxy_block_id_t def);
VOXY_SERVER_EXPORT voxy_light_t voxy_get_block_light_level(ivec3_t position, voxy_light_t def);

/// Block setter.
///
/// It is not possible to also set the light level since that is determined by
/// block info in block registry and light propagation rules.
///
/// In case the block dees not exist (for example if the chunk in which the
/// block resides in has not yet been loaded/generated), return false.
VOXY_SERVER_EXPORT bool voxy_set_block(ivec3_t position, voxy_block_id_t id);

#endif // VOXY_SERVER_CHUNK_BLOCK_MANAGER_H
