#include "mod.h"
#include "weird.h"
#include "entity/player/player.h"

#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/entity_query.h>

#include <libcommon/graphics/mesh.h>
#include <libcommon/graphics/gl.h>
#include <libcommon/graphics/render.h>

#include <libcommon/core/log.h>

#include <stdlib.h>

static entity_id_t weird_entity_id = ENTITY_NONE;
static struct mesh weird_entity_mesh;
static struct gl_texture_2d weird_entity_texture;

void weird_entity_register(void)
{
  struct entity_info entity_info;

  entity_info.mod = MOD;
  entity_info.name = "weird";

  entity_info.hitbox_dimension = fvec3(1.0f, 1.0f, 1.0f);
  entity_info.hitbox_offset = fvec3(0.0f, 0.0f, -0.5f);

  entity_info.on_dispose = NULL;

  entity_info.serialize = NULL;
  entity_info.deserialize = NULL;

  entity_info.on_update = weird_entity_update;
  entity_info.on_render = weird_entity_render;

  weird_entity_id = register_entity_info(entity_info);

  if(mesh_init(&weird_entity_mesh) != 0)
  {
    LOG_ERROR("Failed to initialize mesh for weird entity");
    exit(EXIT_FAILURE);
  }

  if(mesh_load(&weird_entity_mesh, "bin/mod/assets/models/pig.obj") != 0)
  {
    LOG_ERROR("Failed to load mesh for weird entity");
    exit(EXIT_FAILURE);
  }

  if(gl_texture_2d_load(&weird_entity_texture, "bin/mod/assets/models/pig.png") != 0)
  {
    LOG_ERROR("Failed to load texture for weird entity");
    exit(EXIT_FAILURE);
  }
}

entity_id_t weird_entity_id_get(void)
{
  return weird_entity_id;
}

void weird_entity_init(struct entity *entity)
{
  entity->id = weird_entity_id;
  entity->health = 10.0f;
  entity->max_health = 10.0f;
  entity->opaque = NULL;
}

void weird_entity_fini(struct entity *entity)
{
  (void)entity;
}

static fvec3_t normal_to_rotation(fvec3_t normal)
{
  fvec3_t result;
  result.yaw = atan2(-normal.x, normal.y);
  result.pitch = 0.0f;
  result.roll = 0.0f;
  return result;
}

static fvec3_t simple_rotate(fvec3_t v)
{
  float x = v.x;
  float y = v.y;
  v.x = -y;
  v.y = x;
  return v;
}

void weird_entity_update(struct entity *entity, float dt)
{
  if(entity->health <= 0.0f)
  {
    entity->remove = true;
    return;
  }

  const aabb3_t region = aabb3(entity->position, fvec3(100.0f, 100.0f, 100.0f));

  struct entity **entities;
  size_t entity_count;
  world_query_entity(region, &entities, &entity_count);

  fvec3_t impulse = fvec3_zero();
  for(size_t i=0; i<entity_count; ++i)
  {
    if(entities[i] == entity)
      continue;

    if(entities[i]->id == player_entity_id_get())
    {
      const fvec3_t offset = fvec3_sub(entities[i]->position, entity->position);
      const float factor = fvec3_length_squared(offset);
      if(factor >= 5.0f)
      {
        impulse = fvec3_add(impulse, fvec3_mul_scalar(fvec3_div_scalar(offset, factor), 10.0f));
        impulse = fvec3_add(impulse, simple_rotate(fvec3_mul_scalar(fvec3_div_scalar(offset, factor), 3.0f)));
      }

    }
    else if(entities[i]->id == weird_entity_id_get())
    {
      const fvec3_t offset = fvec3_sub(entities[i]->position, entity->position);
      const float factor = fvec3_length_squared(offset);
      if(factor > 0.2f && factor <= 25.0f)
        impulse = fvec3_sub(impulse, fvec3_mul_scalar(fvec3_div_scalar(offset, factor), 3.0f));
    }
  }

  impulse.z = 0.0f;
  impulse = fvec3_normalize(impulse);
  impulse = fvec3_mul_scalar(impulse, 5.0f);
  impulse = fvec3_mul_scalar(impulse, dt);

  entity_apply_impulse(entity, impulse);

  entity->rotation = normal_to_rotation(impulse);

  entity_jump(entity, 10.0f);

}

void weird_entity_render(const struct entity *entity, const struct camera *camera)
{
  const float light = world_get_average_block_light_factor(entity->position, 3.0f);
  render(camera, &weird_entity_mesh, &weird_entity_texture, entity_transform(entity), light);
}

