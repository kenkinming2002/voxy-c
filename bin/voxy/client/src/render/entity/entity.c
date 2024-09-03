#include "entity.h"

#include <stdbool.h>

void entity_renderer_render(struct entity_registry *entity_registry, struct entity_manager *entity_manager, const struct camera *camera)
{
  for(size_t i=0; i<entity_manager->entities.item_count; ++i)
  {
    const struct entity *entity = &entity_manager->entities.items[i];
    if(entity->alive)
    {
      const struct entity_info *info = &entity_registry->infos.items[entity->id]; // FIXME: Potential out of bounds.
      if(info->render)
        info->render(entity, camera);
    }
  }
}

