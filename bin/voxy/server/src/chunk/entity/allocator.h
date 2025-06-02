#ifndef CHUNK_ENTITY_ALLOCATOR_H
#define CHUNK_ENTITY_ALLOCATOR_H

/// Entity allocator module that is well - responsible for allocating entity. We
/// need this over plain malloc(3)/free(3) because we need a persistent id to
/// identify and synchronize entities over network.

#include <voxy/server/registry/entity.h>
#include <voxy/server/chunk/entity/manager.h>

#include "entity.h"

entity_handle_t entity_alloc(void);
void entity_free(entity_handle_t handle);

struct voxy_entity *entity_get(entity_handle_t handle);
struct voxy_entity *entity_get_all(void);

#endif // CHUNK_ENTITY_ALLOCATOR_H
