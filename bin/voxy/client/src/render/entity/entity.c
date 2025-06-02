#include "entity.h"
#include "camera/main.h"
#include "chunk/entity/manager.h"

#include <stb_ds.h>

#include <stdbool.h>

void entity_renderer_render(void)
{
  struct voxy_entity *entities = entity_get_all();
  for(entity_handle_t handle=0; handle<arrlenu(entities); ++handle)
    if(handle != get_main_camera_target())
    {
      const struct voxy_entity *entity = &entities[handle];
      if(entity->alive)
      {
        const struct voxy_entity_info info = voxy_query_entity(entity->id);
        const struct camera camera = get_main_camera();
        if(info.render)
          info.render(entity, &camera);
      }
    }
}

