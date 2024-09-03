#include "entity.h"

#include <stdbool.h>

void entity_renderer_render(struct entity_registry *entity_registry, struct entity_manager *entity_manager, struct camera_manager *camera_manager)
{
  for(entity_handle_t handle=0; handle<entity_manager->entities.item_count; ++handle)
    if(handle != camera_manager->target)
    {
      const struct entity *entity = &entity_manager->entities.items[handle];
      if(entity->alive)
      {
        const struct entity_info *info = &entity_registry->infos.items[entity->id]; // FIXME: Potential out of bounds.
        if(info->render)
          info->render(entity, &camera_manager->camera);
      }
    }
}

