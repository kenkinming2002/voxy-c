#include <voxy/server/context.h>
#include <voxy/server/entity/entity.h>

static void player_entity_update(const struct voxy_entity *entity, const struct voxy_context *context)
{
  const ivec3_t center = ivec3_div_scalar(fvec3_as_ivec3_round(voxy_entity_get_position(entity)), 16);
  const int radius = 10;
  for(int z=center.z-radius; z<=center.z+radius; ++z)
    for(int y=center.y-radius; y<=center.y+radius; ++y)
      for(int x=center.x-radius; x<=center.x+radius; ++x)
      {
        const ivec3_t position = ivec3(x, y, z);
        if(ivec3_length_squared(ivec3_sub(position, center)) <= radius * radius)
          chunk_manager_add_active_chunk(context->chunk_manager, position);
      }
}


void *mod_create_instance(struct voxy_context *context)
{
  voxy_block_registry_register_block(context->block_registry, (struct voxy_block_info){
    .mod = "base",
    .name = "air",
  });

  voxy_block_registry_register_block(context->block_registry, (struct voxy_block_info){
    .mod = "base",
    .name = "ether",
  });

  voxy_block_registry_register_block(context->block_registry, (struct voxy_block_info){
    .mod = "base",
    .name = "stone",
  });

  voxy_block_registry_register_block(context->block_registry, (struct voxy_block_info){
    .mod = "base",
    .name = "grass",
  });

  voxy_entity_registry_register_entity(context->entity_registry, (struct voxy_entity_info) {
    .mod = "base",
    .name = "player",
    .update = player_entity_update,
  });

  voxy_entity_registry_register_entity(context->entity_registry, (struct voxy_entity_info) {
    .mod = "base",
    .name = "dummy",
    .update = NULL,
  });

  return NULL;
}

void mod_destroy_instance(struct voxy_context *context, void *instance)
{
  (void)context;
  (void)instance;
}

