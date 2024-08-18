#include "mod.h"
#include "item.h"

#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/render/assets.h>

#include <libcommon/graphics/gl.h>
#include <libcommon/graphics/render.h>

#include <stdlib.h>

static entity_id_t item_entity_id = ENTITY_NONE;
static struct mesh item_entity_mesh;

void item_entity_register(void)
{
  struct entity_info entity_info;

  entity_info.mod = MOD;
  entity_info.name = "item";

  entity_info.hitbox_dimension = fvec3(0.4f, 0.4f, 0.4f);
  entity_info.hitbox_offset = fvec3_zero();

  entity_info.on_dispose = NULL;

  entity_info.serialize = item_entity_serialize;
  entity_info.deserialize = item_entity_deserialize;

  entity_info.on_update = item_entity_update;
  entity_info.on_render = item_entity_render;

  item_entity_id = register_entity_info(entity_info);

  if(mesh_init(&item_entity_mesh) != 0)
  {
    LOG_ERROR("Failed to initialize mesh for item entity");
    exit(EXIT_FAILURE);
  }

  if(mesh_load(&item_entity_mesh, "mod/assets/models/item.obj") != 0)
  {
    LOG_ERROR("Failed to load mesh for item entity");
    exit(EXIT_FAILURE);
  }
}

entity_id_t item_entity_id_get(void)
{
  return item_entity_id;
}

void item_entity_init(struct entity *entity, struct item item)
{
  struct item_opaque *opaque = malloc(sizeof *opaque);
  opaque->item = item;

  entity->id = item_entity_id;
  entity->health = INFINITY;
  entity->max_health = INFINITY;
  entity->opaque = opaque;
}

void item_entity_fini(struct entity *entity)
{
  free(entity->opaque);
}

int item_entity_serialize(const struct entity *entity, struct serializer *serializer)
{
  const struct item_opaque *opaque = entity->opaque;
  SERIALIZE(serializer, opaque->item);
  return 0;
}

int item_entity_deserialize(struct entity *entity, struct deserializer *deserializer)
{
  struct item_opaque *opaque = malloc(sizeof *opaque);
  entity->opaque = opaque;
  DESERIALIZE(deserializer, opaque->item);
  return 0;
}

void item_entity_update(struct entity *entity, float dt)
{
  (void)entity;
  (void)dt;
}

static fvec3_t normal_to_rotation(fvec3_t normal)
{
  fvec3_t result;
  result.yaw = atan2(-normal.x, normal.y);
  result.pitch = atan2(normal.z,sqrtf(normal.x*normal.x+normal.y*normal.y));
  result.roll = 0.0f;
  return result;
}

void item_entity_render(const struct entity *entity, const struct camera *camera)
{
  transform_t transform = entity_transform(entity);
  transform.rotation = normal_to_rotation(fvec3_neg(transform_forward(camera->transform)));

  const struct item_opaque *opaque = entity->opaque;
  const struct gl_texture_2d *texture = assets_get_item_texture(opaque->item.id);

  const float light = world_get_average_block_light_factor(entity->position, 3.0f);
  render(camera, &item_entity_mesh, texture, transform, light);
}
