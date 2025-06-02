#include <voxy/client/context.h>

#include <voxy/client/registry/block.h>
#include <voxy/client/registry/entity.h>

#include <voxy/client/entity/entity.h>

#include <libgfx/gl.h>
#include <libgfx/mesh.h>
#include <libgfx/render.h>

static void test_render(const struct voxy_entity *entity, const struct camera *camera)
{
  // FIXME: We are currently hardcoding and loading assets on first use.
  static bool initialized = false;
  static struct mesh mesh;
  static struct gl_texture_2d texture;
  if(!initialized)
  {
    mesh_init(&mesh);
    mesh_load(&mesh, "bin/mod/assets/models/pig.obj");
    gl_texture_2d_load(&texture, "bin/mod/assets/models/pig.png");
    initialized = true;
  }

  transform_t transform;
  transform.translation = voxy_entity_get_position(entity);
  transform.rotation = voxy_entity_get_rotation(entity);
  render(camera, &mesh, &texture, transform, 1.0f);
}

void *mod_create_instance(struct voxy_context *context)
{
  voxy_register_block((struct voxy_block_info){
    .mod = "base",
    .name = "air",
    .type = VOXY_BLOCK_TYPE_INVISIBLE,
    .textures = {0},
  });

  voxy_register_block((struct voxy_block_info){
    .mod = "base",
    .name = "ether",
    .type = VOXY_BLOCK_TYPE_INVISIBLE,
    .textures = {0},
  });

  voxy_register_block((struct voxy_block_info){
    .mod = "base",
    .name = "stone",
    .type = VOXY_BLOCK_TYPE_OPAQUE,
    .textures[DIRECTION_LEFT]   = "bin/mod/assets/textures/stone.png",
    .textures[DIRECTION_RIGHT]  = "bin/mod/assets/textures/stone.png",
    .textures[DIRECTION_BACK]   = "bin/mod/assets/textures/stone.png",
    .textures[DIRECTION_FRONT]  = "bin/mod/assets/textures/stone.png",
    .textures[DIRECTION_BOTTOM] = "bin/mod/assets/textures/stone.png",
    .textures[DIRECTION_TOP]    = "bin/mod/assets/textures/stone.png",
  });

  voxy_register_block((struct voxy_block_info){
    .mod = "base",
    .name = "grass",
    .type = VOXY_BLOCK_TYPE_OPAQUE,
    .textures[DIRECTION_LEFT]   = "bin/mod/assets/textures/grass_side.png",
    .textures[DIRECTION_RIGHT]  = "bin/mod/assets/textures/grass_side.png",
    .textures[DIRECTION_BACK]   = "bin/mod/assets/textures/grass_side.png",
    .textures[DIRECTION_FRONT]  = "bin/mod/assets/textures/grass_side.png",
    .textures[DIRECTION_BOTTOM] = "bin/mod/assets/textures/grass_bottom.png",
    .textures[DIRECTION_TOP]    = "bin/mod/assets/textures/grass_top.png",
  });

  voxy_register_block((struct voxy_block_info){
    .mod = "base",
    .name = "water",
    .type = VOXY_BLOCK_TYPE_TRANSPARENT,
    .textures[DIRECTION_LEFT]   = "bin/mod/assets/textures/water.png",
    .textures[DIRECTION_RIGHT]  = "bin/mod/assets/textures/water.png",
    .textures[DIRECTION_BACK]   = "bin/mod/assets/textures/water.png",
    .textures[DIRECTION_FRONT]  = "bin/mod/assets/textures/water.png",
    .textures[DIRECTION_BOTTOM] = "bin/mod/assets/textures/water.png",
    .textures[DIRECTION_TOP]    = "bin/mod/assets/textures/water.png",
  });

  voxy_register_entity((struct voxy_entity_info) {
    .mod = "base",
    .name = "player",
    .render = test_render,
  });

  voxy_register_entity((struct voxy_entity_info) {
    .mod = "base",
    .name = "dummy",
    .render = test_render,
  });

  return NULL;
}

void mod_destroy_instance(struct voxy_context *context, void *instance)
{
  (void)context;
  (void)instance;
}

