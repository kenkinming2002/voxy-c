#include "entity.h"
#include "camera/main.h"

#include <stb_ds.h>

#include <stdbool.h>

void entity_renderer_render(struct entity_manager *entity_manager)
{
  for(entity_handle_t handle=0; handle<arrlenu(entity_manager->entities); ++handle)
    if(handle != get_main_camera_target())
    {
      const struct voxy_entity *entity = &entity_manager->entities[handle];
      if(entity->alive)
      {
        const struct voxy_entity_info info = voxy_query_entity(entity->id);
        const struct camera camera = get_main_camera();
        if(info.render)
          info.render(entity, &camera);
      }
    }
}

