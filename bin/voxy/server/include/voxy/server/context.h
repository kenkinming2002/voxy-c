#ifndef VOXY_SERVER_CONTEXT_H
#define VOXY_SERVER_CONTEXT_H

#include "block/registry.h"
#include "entity/registry.h"
#include "chunk/manager.h"

/// Context.
///
/// This bundles together all the data that may be accessed by a mod.
struct voxy_context
{
  struct voxy_block_registry *block_registry;
  struct voxy_entity_registry *entity_registry;
  struct voxy_chunk_manager *chunk_manager;
};

#endif // VOXY_SERVER_CONTEXT_H
