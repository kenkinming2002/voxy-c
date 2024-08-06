#include "mod.h"
#include "weird.h"

#include <libcommon/graphics/mesh.h>
#include <libcommon/graphics/gl.h>
#include <libcommon/graphics/render.h>

#include <libcommon/core/log.h>

#include <stdlib.h>

static float randf(float low, float high)
{
  return low + (float)rand() / (float)RAND_MAX * (high - low);
}

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

  entity_info.on_save = NULL;
  entity_info.on_load = NULL;

  entity_info.on_update = weird_entity_update;
  entity_info.on_render = weird_entity_render;

  weird_entity_id = register_entity_info(entity_info);

  if(mesh_load(&weird_entity_mesh, "mod/assets/models/pig.obj") != 0)
  {
    LOG_ERROR("Failed to load mesh for weird entity");
    exit(EXIT_FAILURE);
  }

  if(gl_texture_2d_load(&weird_entity_texture, "mod/assets/models/pig.png") != 0)
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
  entity->opaque = NULL;
}

void weird_entity_fini(struct entity *entity)
{
  (void)entity;
}

void weird_entity_update(struct entity *entity, float dt)
{
  entity_jump(entity, 10.0f);
  entity_move(entity, fvec2(randf(-1.0f, 1.0f), randf(-1.0f, 1.0f)), 30.0f, dt);
}

void weird_entity_render(const struct entity *entity, const struct camera *camera)
{
  render_model(*camera, entity_transform(entity), weird_entity_mesh, weird_entity_texture);
}

