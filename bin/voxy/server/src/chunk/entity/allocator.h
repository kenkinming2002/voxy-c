#ifndef CHUNK_ENTITY_ALLOCATOR_H
#define CHUNK_ENTITY_ALLOCATOR_H

#include "registry/entity.h"
#include "entity.h"

#include <voxy/server/chunk/entity/manager.h>

/// Entity allocator.
///
/// This takes care of allocating a unique handle for each entity.
struct entity_allocator
{
  struct voxy_entity *entities;
  entity_handle_t *orphans;
};

void entity_allocator_init(struct entity_allocator *entity_allocator);
void entity_allocator_fini(struct entity_allocator *entity_allocator);

entity_handle_t entity_allocator_alloc(struct entity_allocator *entity_allocator);
void entity_allocator_free(struct entity_allocator *entity_allocator, struct voxy_entity_registry *entity_registry, entity_handle_t handle);

struct voxy_entity *entity_allocator_get(struct entity_allocator *entity_allocator, entity_handle_t handle);

#endif // CHUNK_ENTITY_ALLOCATOR_H
