#include <voxy/config.h>

#include <voxy/server/registry/block.h>

#include <voxy/server/chunk/manager.h>
#include <voxy/server/chunk/block/group.h>
#include <voxy/server/chunk/block/generator.h>
#include <voxy/server/chunk/entity/entity.h>

#include <voxy/server/chunk/block/manager.h>

#include <voxy/server/player/player.h>
#include <voxy/server/player/manager.h>

#include <libmath/matrix_transform.h>
#include <libmath/ray_cast.h>
#include <libmath/noise.h>

#include <stdio.h>

#ifdef VOXY_STRESS_TEST
#define PAN_SPEED 0.002f
#define MOVE_SPEED_GROUND 800.0f
#define MOVE_SPEED_AIR 300.0f
#define JUMP_STRENGTH 50.0f
#else
#define PAN_SPEED 0.002f
#define MOVE_SPEED_GROUND 8.0f
#define MOVE_SPEED_AIR 3.0f
#define JUMP_STRENGTH 5.5f
#endif

static void generate_block(seed_t seed, ivec3_t chunk_position, voxy_block_id_t blocks[VOXY_CHUNK_WIDTH][VOXY_CHUNK_WIDTH][VOXY_CHUNK_WIDTH])
{
  // Generate height map
  float heights[VOXY_CHUNK_WIDTH][VOXY_CHUNK_WIDTH];
  for(int y=0; y<VOXY_CHUNK_WIDTH; ++y)
    for(int x=0; x<VOXY_CHUNK_WIDTH; ++x)
    {
      const ivec2_t position = ivec2_add(ivec2_mul_scalar(ivec2(chunk_position.x, chunk_position.y), VOXY_CHUNK_WIDTH), ivec2(x, y));
      heights[y][x] = noise_perlin2_ex(seed, ivec2_as_fvec2(position), 0.008f, 2.0f, 0.3f, 8) * 50.0f;
    }

  // Populate block ids and light levels.
  for(int z=0; z<VOXY_CHUNK_WIDTH; ++z)
    for(int y=0; y<VOXY_CHUNK_WIDTH; ++y)
      for(int x=0; x<VOXY_CHUNK_WIDTH; ++x)
      {
        const int height = VOXY_CHUNK_WIDTH * chunk_position.z + z;
        blocks[z][y][x] = height > 64 ? 1
          : height > heights[y][x] ? (height < 0 ? 4 : 0)
          : height > heights[y][x] - 1 ? 3
          : 2;
      }
}

static void on_new_player(struct voxy_player *player)
{
  // FIXME: We need to load the player from disk.
  const fvec3_t position = fvec3(0.0f, 0.0f, 50.0f);
  const fvec3_t rotation = fvec3_zero();
  const entity_handle_t handle = voxy_entity_spawn(0, position, rotation, voxy_player_get_weak(player));
  voxy_player_set_camera_follow_entity(player, handle);
}

static bool player_entity_update(struct voxy_entity *entity, float dt)
{
  // The stored opaque pointer could become NULL after a
  // serialization/deserialization cycle, possibly after server restart.
  struct voxy_player *player = voxy_entity_get_opaque(entity);
  if(!player)
    return false;

  // Check if we are still connected.
  if(!voxy_player_upgrade(player))
  {
    voxy_player_put_weak(player);
    return false;
  }

  // Chunk Loading
  {
    const ivec3_t center = ivec3_div_scalar(fvec3_as_ivec3_round(voxy_entity_get_position(entity)), VOXY_CHUNK_WIDTH);
    const int major_radius = 10;
    const int minor_radius = 15;
    voxy_add_chunk_region_sphere(center, major_radius, minor_radius);
  }

  // Movement controller
  {
    const fvec2_t pan = voxy_player_get_pan_input(player);
    const fvec3_t old_rotation = voxy_entity_get_rotation(entity);
    const fvec3_t rotation = fvec3(old_rotation.x + pan.x * PAN_SPEED, old_rotation.y - pan.y * PAN_SPEED, old_rotation.z);
    voxy_entity_set_rotation(entity, rotation);

    const fvec3_t movement = voxy_player_get_movement_input(player);
    const fvec4_t direction4 = fmat4_mul_vec(fmat4_rotate(rotation), fvec4(movement.x, movement.y, 0.0f, 1.0f));
    const fvec3_t direction =  fvec3_normalize(fvec3(direction4.x, direction4.y, 0.0f));
    voxy_entity_apply_impulse(entity, fvec3_mul_scalar(direction, (voxy_entity_is_grounded(entity) ? MOVE_SPEED_GROUND : MOVE_SPEED_AIR) * dt));

    if(voxy_entity_is_grounded(entity) && movement.z > 0.5f)
      voxy_entity_apply_impulse(entity, fvec3(0.0f, 0.0f, JUMP_STRENGTH));
  }

  // Breaking block
  if(voxy_player_get_left_mouse_button_input(player))
  {
    const transform_t transform = voxy_entity_get_transform(entity);
    const fvec3_t position = transform.translation;
    const fvec3_t direction = transform_forward(transform);

    struct ray_cast ray_cast;
    for(ray_cast_init(&ray_cast, position); ray_cast.distance < 10.0f; ray_cast_step(&ray_cast, direction))
    {
      const uint8_t id = voxy_get_block_id(ray_cast.iposition, 0xFF);
      if(id == 0xFF)
        continue;

      const struct voxy_block_info info = voxy_query_block(id);
      if(!info.collide)
        continue;

      voxy_set_block(ray_cast.iposition, 0);
      break;
    }
  }

  voxy_player_put(player);
  return true;
}

int mod_init(void)
{
  voxy_set_generate_block(generate_block);

  voxy_register_block((struct voxy_block_info){
    .mod = "base",
    .name = "air",
    .collide = false,
    .light_level = 0,
  });

  voxy_register_block((struct voxy_block_info){
    .mod = "base",
    .name = "ether",
    .collide = false,
    .light_level = 15,
  });

  voxy_register_block((struct voxy_block_info){
    .mod = "base",
    .name = "stone",
    .collide = true,
    .light_level = 0,
  });

  voxy_register_block((struct voxy_block_info){
    .mod = "base",
    .name = "grass",
    .collide = true,
    .light_level = 0,
  });

  voxy_register_block((struct voxy_block_info){
    .mod = "base",
    .name = "water",
    .collide = false,
    .light_level = 0,
  });

  voxy_register_entity((struct voxy_entity_info) {
    .mod = "base",
    .name = "player",
    .update = player_entity_update,
    .hitbox_dimension = fvec3(0.8f, 0.8f, 1.9f),
    .hitbox_offset = fvec3(0.0f, 0.0f, -0.5f),
    .destroy_opaque = voxy_entity_destroy_opaque_default,
    .serialize_opaque = voxy_entity_serialize_opaque_default,
    .deserialize_opaque = voxy_entity_deserialize_opaque_default,
  });

  voxy_register_entity((struct voxy_entity_info) {
    .mod = "base",
    .name = "dummy",
    .update = NULL,
    .hitbox_dimension = fvec3(0.8f, 0.8f, 1.9f),
    .hitbox_offset = fvec3(0.0f, 0.0f, -0.5f),
    .destroy_opaque = voxy_entity_destroy_opaque_default,
    .serialize_opaque = voxy_entity_serialize_opaque_default,
    .deserialize_opaque = voxy_entity_deserialize_opaque_default,
  });

  voxy_set_on_new_player(on_new_player);

  return 0;
}

