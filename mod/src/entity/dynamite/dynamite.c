#include "mod.h"
#include "dynamite.h"

#include "block/empty/empty.h"

#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/render/assets.h>

#include <libcommon/graphics/gl.h>
#include <libcommon/graphics/render.h>

#include <stdlib.h>

static entity_id_t dynamite_entity_id = ENTITY_NONE;
static struct mesh dynamite_entity_mesh;

void dynamite_entity_register(void)
{
  struct entity_info entity_info;

  entity_info.mod = MOD;
  entity_info.name = "dynamite";

  entity_info.hitbox_dimension = fvec3(0.4f, 0.4f, 0.4f);
  entity_info.hitbox_offset = fvec3_zero();

  entity_info.on_dispose = NULL;

  entity_info.on_save = dynamite_entity_save;
  entity_info.on_load = dynamite_entity_load;

  entity_info.on_update = dynamite_entity_update;
  entity_info.on_render = dynamite_entity_render;

  dynamite_entity_id = register_entity_info(entity_info);

  if(mesh_load(&dynamite_entity_mesh, "mod/assets/models/item.obj") != 0)
  {
    LOG_ERROR("Failed to load mesh for dynamite entity");
    exit(EXIT_FAILURE);
  }
}

entity_id_t dynamite_entity_id_get(void)
{
  return dynamite_entity_id;
}

void dynamite_entity_init(struct entity *entity, float fuse)
{
  struct dynamite_opaque *opaque = malloc(sizeof *opaque);
  opaque->fuse = fuse;
  entity->id = dynamite_entity_id;
  entity->opaque = opaque;
}

void dynamite_entity_fini(struct entity *entity)
{
  free(entity->opaque);
}

bool dynamite_entity_save(const struct entity *entity, FILE *file)
{
  const struct dynamite_opaque *opaque = entity->opaque;
  return fwrite(opaque, sizeof *opaque, 1, file) == 1;
}

bool dynamite_entity_load(struct entity *entity, FILE *file)
{
  struct dynamite_opaque *opaque = malloc(sizeof *opaque);
  entity->opaque = opaque;
  return fread(opaque, sizeof *opaque, 1, file) == 1;
}

void dynamite_entity_update(struct entity *entity, float dt)
{
  struct dynamite_opaque *opaque = entity->opaque;
  opaque->fuse -= dt;
  if(opaque->fuse < 0.0f)
  {
    const fvec3_t center = entity->position;
    const float radius = 5.0f;

    const ivec3_t i_center = fvec3_as_ivec3_round(center);
    const int i_radius = ceilf(radius);

    for(int z=-i_radius; z<=i_radius; ++z)
      for(int y=-i_radius; y<=i_radius; ++y)
        for(int x=-i_radius; x<=i_radius; ++x)
        {
          const ivec3_t position = ivec3_add(i_center, ivec3(x, y, z));
          const fvec3_t offset = fvec3_sub(ivec3_as_fvec3(position), center);
          if(fvec3_length_squared(offset) <= radius * radius)
            world_set_block(position, empty_block_id_get(), entity);
        }

    entity->remove = true;
  }
}

static fvec3_t normal_to_rotation(fvec3_t normal)
{
  fvec3_t result;
  result.yaw = atan2(-normal.x, normal.y);
  result.pitch = atan2(normal.z,sqrtf(normal.x*normal.x+normal.y*normal.y));
  result.roll = 0.0f;
  return result;
}

static struct gl_texture_2d get_dynamite_texture(void)
{
  static bool initialized;
  static struct gl_texture_2d texture;
  if(!initialized)
  {
    gl_texture_2d_load(&texture, "mod/assets/textures/dynamite_item.png");
    initialized = true;
  }
  return texture;
}

void dynamite_entity_render(const struct entity *entity, const struct camera *camera)
{
  transform_t transform = entity_transform(entity);
  transform.rotation = normal_to_rotation(fvec3_neg(transform_forward(camera->transform)));

  const struct gl_texture_2d texture = get_dynamite_texture();
  const float light = world_get_average_block_light_factor(entity->position, 3.0f);
  render_model(*camera, transform, dynamite_entity_mesh, texture, light);
}
