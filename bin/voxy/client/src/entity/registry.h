#ifndef ENTITY_REGISTRY_H
#define ENTITY_REGISTRY_H

#include "info.h"

#include <stdint.h>

typedef uint8_t entity_id_t;

struct entity_registry
{
  struct entity_infos infos;
};

void entity_registry_init(struct entity_registry *registry);
void entity_registry_fini(struct entity_registry *registry);

entity_id_t entity_registry_register_entity(struct entity_registry *registry, struct entity_info entity_info);
struct entity_info entity_registry_query_entity(struct entity_registry *registry, entity_id_t id);

#endif // ENTITY_REGISTRY_H
