#ifndef VOXY_SCENE_MAIN_GAME_TYPES_ENTITY_H
#define VOXY_SCENE_MAIN_GAME_TYPES_ENTITY_H

#include <voxy/scene/main_game/types/registry.h>

#include <libcommon/math/vector.h>
#include <libcommon/math/transform.h>
#include <libcommon/math/aabb.h>
#include <libcommon/utils/dynamic_array.h>

#include <stdbool.h>

struct entity
{
  entity_id_t id;

  fvec3_t position;
  fvec3_t velocity;
  fvec3_t rotation;

  float max_health;
  float health;

  float max_height;
  bool grounded;

  bool remove;

  void *opaque;
};
DYNAMIC_ARRAY_DEFINE(entities, struct entity);

void entity_init(struct entity *entity, fvec3_t position, fvec3_t rotation, fvec3_t velocity, float max_health, float health);

transform_t entity_transform(const struct entity *entity);
aabb3_t entity_hitbox(const struct entity *entity);

fvec3_t entity_local_to_global(struct entity *entity, fvec3_t vec);

void entity_apply_impulse(struct entity *entity, fvec3_t impulse);
void entity_move(struct entity *entity, fvec2_t direction, float speed, float dt);
void entity_jump(struct entity *entity, float strength);

bool entity_ray_cast(struct entity *entity, float distance, ivec3_t *position, ivec3_t *normal);

#endif // VOXY_SCENE_MAIN_GAME_TYPES_ENTITY_H
