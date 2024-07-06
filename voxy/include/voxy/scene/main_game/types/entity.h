#ifndef VOXY_SCENE_MAIN_GAME_TYPES_ENTITY_H
#define VOXY_SCENE_MAIN_GAME_TYPES_ENTITY_H

#include <voxy/math/vector.h>
#include <voxy/math/transform.h>

#include <voxy/scene/main_game/types/registry.h>

#include <stdbool.h>

struct entity
{
  entity_id_t id;

  fvec3_t position;
  fvec3_t velocity;
  fvec3_t rotation;

  bool grounded;

  void *opaque;
};

fvec3_t entity_local_to_global(struct entity *entity, fvec3_t vec);

void entity_apply_impulse(struct entity *entity, fvec3_t impulse);
void entity_move(struct entity *entity, fvec2_t direction, float speed, float dt);
void entity_jump(struct entity *entity, float strength);

bool entity_ray_cast(struct entity *entity, float distance, ivec3_t *position, ivec3_t *normal);

// Check if two entity collision box intersect.
bool entity_intersect(struct entity *entity1, struct entity *entity2);

#endif // VOXY_SCENE_MAIN_GAME_TYPES_ENTITY_H
