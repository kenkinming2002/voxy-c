#include <voxy/scene/main_game/types/entities.h>

void entities_init(struct entities *entities)
{
  DYNAMIC_ARRAY_INIT(*entities);
}

void entities_fini(struct entities *entities)
{
  for(size_t i=0; i<entities->item_count; ++i)
    entity_fini(&entities->items[i]);

  DYNAMIC_ARRAY_CLEAR(*entities);
}

void entities_append(struct entities *entities, struct entity entity)
{
  DYNAMIC_ARRAY_APPEND(*entities, entity);
}
