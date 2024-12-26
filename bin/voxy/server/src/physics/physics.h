#ifndef PHYSICS_PHYSICS_H
#define PHYSICS_PHYSICS_H

#include "registry/block.h"
#include "registry/entity.h"

#include "chunk/manager.h"
#include "entity/manager.h"

void physics_update(struct voxy_block_registry *block_registry,
                    struct voxy_entity_registry *entity_registry,
                    struct voxy_chunk_manager *chunk_manager,
                    struct voxy_entity_manager *entity_manager, float dt);

#endif // PHYSICS_PHYSICS_H
