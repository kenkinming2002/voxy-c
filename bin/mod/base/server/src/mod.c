#include <voxy/server/context.h>
#include <voxy/server/entity/entity.h>
#include <voxy/server/player/player.h>

#include <libcommon/math/matrix_transform.h>

#define FIXED_DT (1/50.0f)

static void on_new_player(struct voxy_player *player, const struct voxy_context *context)
{
  // FIXME: We need to load the player from disk.
  const fvec3_t position = fvec3_zero();
  const fvec3_t rotation = fvec3_zero();
  const entity_handle_t handle = voxy_entity_manager_spawn(context->entity_manager, 0, position, rotation, voxy_player_get_weak(player), context->server);
  voxy_player_set_camera_follow_entity(player, handle);
}

static void player_entity_update(struct voxy_entity *entity, float dt, const struct voxy_context *context)
{
  struct voxy_player *player = voxy_entity_get_opaque(entity);
  if(voxy_player_upgrade(player))
  {
    const ivec3_t center = ivec3_div_scalar(fvec3_as_ivec3_round(voxy_entity_get_position(entity)), 16);
    const int radius = 10;
    for(int z=center.z-radius; z<=center.z+radius; ++z)
      for(int y=center.y-radius; y<=center.y+radius; ++y)
        for(int x=center.x-radius; x<=center.x+radius; ++x)
        {
          const ivec3_t position = ivec3(x, y, z);
          if(ivec3_length_squared(ivec3_sub(position, center)) <= radius * radius)
            voxy_chunk_manager_add_active_chunk(context->chunk_manager, position);
        }

    fvec3_t position = voxy_entity_get_position(entity);
    fvec3_t rotation = voxy_entity_get_rotation(entity);

    const fvec3_t movement = voxy_player_get_movement_input(player);
    const fvec2_t pan = voxy_player_get_pan_input(player);

    fvec4_t offset4 = fmat4_mul_vec(fmat4_rotate(rotation), fvec4(movement.x, movement.y, movement.z, 1.0f));
    fvec3_t offset = fvec3(offset4.x, offset4.y, offset4.z);
    offset = fvec3_mul_scalar(offset, dt);
    offset = fvec3_mul_scalar(offset, 100.0f);
    position = fvec3_add(position, offset);

    rotation.yaw   +=  pan.x * 0.002f;
    rotation.pitch += -pan.y * 0.002f;

    voxy_entity_set_position(entity, position);
    voxy_entity_set_rotation(entity, rotation);
  }
  else
  {
    // FIXME: We need to save the player to disk.
    voxy_player_put_weak(player);
  }
}

void *mod_create_instance(struct voxy_context *context)
{
  voxy_block_registry_register_block(context->block_registry, (struct voxy_block_info){
    .mod = "base",
    .name = "air",
    .collide = false,
  });

  voxy_block_registry_register_block(context->block_registry, (struct voxy_block_info){
    .mod = "base",
    .name = "ether",
    .collide = false,
  });

  voxy_block_registry_register_block(context->block_registry, (struct voxy_block_info){
    .mod = "base",
    .name = "stone",
    .collide = true,
  });

  voxy_block_registry_register_block(context->block_registry, (struct voxy_block_info){
    .mod = "base",
    .name = "grass",
    .collide = true,
  });

  voxy_entity_registry_register_entity(context->entity_registry, (struct voxy_entity_info) {
    .mod = "base",
    .name = "player",
    .update = player_entity_update,
    .hitbox_dimension = fvec3(0.8f, 0.8f, 1.9f),
    .hitbox_offset = fvec3(0.0f, 0.0f, -0.5f),
  });

  voxy_entity_registry_register_entity(context->entity_registry, (struct voxy_entity_info) {
    .mod = "base",
    .name = "dummy",
    .update = NULL,
    .hitbox_dimension = fvec3(0.8f, 0.8f, 1.9f),
    .hitbox_offset = fvec3(0.0f, 0.0f, -0.5f),
  });

  voxy_player_manager_set_on_new_player(context->player_manager, on_new_player);

  return NULL;
}

void mod_destroy_instance(struct voxy_context *context, void *instance)
{
  (void)context;
  (void)instance;
}

