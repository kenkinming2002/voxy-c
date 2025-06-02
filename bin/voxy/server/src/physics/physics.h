#ifndef PHYSICS_PHYSICS_H
#define PHYSICS_PHYSICS_H

#include "chunk/block/manager.h"
#include "chunk/entity/manager.h"

void physics_update(struct voxy_block_manager *block_manager,
                    struct voxy_entity_manager *entity_manager, float dt);

#endif // PHYSICS_PHYSICS_H
