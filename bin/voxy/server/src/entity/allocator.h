#ifndef ENTITY_ALLOCATOR_H
#define ENTITY_ALLOCATOR_H

#include "registry/entity.h"
#include "entity.h"

#include <voxy/server/entity/manager.h>
#include <libcommon/utils/dynamic_array.h>

/// Entity allocator.
///
/// This takes care of allocating a unique handle for each entity.
struct entity_allocator
{
  DYNAMIC_ARRAY_DEFINE(,struct voxy_entity) entities;
  DYNAMIC_ARRAY_DEFINE(,entity_handle_t) orphans;
};

void entity_allocator_init(struct entity_allocator *entity_allocator);
void entity_allocator_fini(struct entity_allocator *entity_allocator);

entity_handle_t entity_allocator_alloc(struct entity_allocator *entity_allocator);
void entity_allocator_free(struct entity_allocator *entity_allocator, struct voxy_entity_registry *entity_registry, entity_handle_t handle);

struct voxy_entity *entity_allocator_get(struct entity_allocator *entity_allocator, entity_handle_t handle);

#endif // ENTITY_ALLOCATOR_H
