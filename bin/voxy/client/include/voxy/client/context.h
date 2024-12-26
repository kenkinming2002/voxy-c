#ifndef VOXY_CLIENT_CONTEXT_H
#define VOXY_CLIENT_CONTEXT_H

#include "registry/block.h"
#include "registry/entity.h"

/// Context.
///
/// This bundles together all the data that may be accessed by a mod.
struct voxy_context
{
  struct voxy_block_registry *block_registry;
  struct voxy_entity_registry *entity_registry;
};

#endif // VOXY_CLIENT_CONTEXT_H
