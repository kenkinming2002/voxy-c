#include "entity.h"

#include <stdbool.h>

void entity_renderer_render(struct voxy_entity_registry *entity_registry, struct entity_manager *entity_manager, struct camera_manager *camera_manager)
{
  for(entity_handle_t handle=0; handle<entity_manager->entities.item_count; ++handle)
    if(handle != camera_manager->target)
    {
      const struct voxy_entity *entity = &entity_manager->entities.items[handle];
      if(entity->alive)
      {
        const struct voxy_entity_info info = voxy_entity_registry_query_entity(entity_registry, entity->id);
        if(info.render)
          info.render(entity, &camera_manager->camera);
      }
    }
}

