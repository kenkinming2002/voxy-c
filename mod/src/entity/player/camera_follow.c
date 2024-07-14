#include "camera_follow.h"

#include <voxy/scene/main_game/states/camera.h>

#include <voxy/graphics/camera.h>
#include <voxy/core/window.h>

void player_entity_update_camera_follow(struct entity *entity, float dt)
{
  struct player_opaque *opaque = entity->opaque;

  world_camera.transform.translation = entity->position;
  world_camera.transform.rotation = entity->rotation;
  if(opaque->third_person)
    world_camera.transform.translation = fvec3_sub(world_camera.transform.translation, entity_local_to_global(entity, fvec3(0.0f, 10.0f, 0.0f)));

  world_camera.fovy   = M_PI / 2.0f;
  world_camera.near   = 0.1f;
  world_camera.far    = 1000.0f;
  world_camera.aspect = (float)window_size.x / (float)window_size.y;
}

