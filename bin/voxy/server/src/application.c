#include "application.h"
#include "config.h"

#include <stdio.h>
#include <time.h>

#include <libcommon/core/log.h>

static void player_entity_update(const struct entity *entity, struct chunk_manager *chunk_manager)
{
  const ivec3_t center = ivec3_div_scalar(fvec3_as_ivec3_round(entity->position), VOXY_CHUNK_WIDTH);
  const int radius = 10;
  for(int z=center.z-radius; z<=center.z+radius; ++z)
    for(int y=center.y-radius; y<=center.y+radius; ++y)
      for(int x=center.x-radius; x<=center.x+radius; ++x)
      {
        const ivec3_t position = ivec3(x, y, z);
        if(ivec3_length_squared(ivec3_sub(position, center)) <= radius * radius)
          chunk_manager_add_active_chunk(chunk_manager, position);
      }
}

int application_init(struct application *application, int argc, char *argv[])
{
  if(argc != 2)
  {
    fprintf(stderr, "Usage: %s SERVICE", argv[0]);
    return -1;
  }

  block_registry_init(&application->block_registry);
  entity_registry_init(&application->entity_registry);

  block_registry_register_block(&application->block_registry, (struct block_info){
    .mod = "base",
    .name = "air",
  });

  block_registry_register_block(&application->block_registry, (struct block_info){
    .mod = "base",
    .name = "ether",
  });

  block_registry_register_block(&application->block_registry, (struct block_info){
    .mod = "base",
    .name = "stone",
  });

  block_registry_register_block(&application->block_registry, (struct block_info){
    .mod = "base",
    .name = "grass",
  });

  entity_registry_register_entity(&application->entity_registry, (struct entity_info) {
    .mod = "base",
    .name = "player",
    .update = player_entity_update,
  });

  entity_registry_register_entity(&application->entity_registry, (struct entity_info) {
    .mod = "base",
    .name = "dummy",
    .update = NULL,
  });

  if(!(application->server = libnet_server_create(argv[1], FIXED_DT * 1e9)))
    goto error0;

  chunk_manager_init(&application->chunk_manager);
  chunk_generator_init(&application->chunk_generator, time(NULL));

  entity_manager_init(&application->entity_manager);

  libnet_server_set_opaque(application->server, application);
  libnet_server_set_on_update(application->server, application_on_update);
  libnet_server_set_on_client_connected(application->server, application_on_client_connected);
  libnet_server_set_on_client_disconnected(application->server, application_on_client_disconnected);
  libnet_server_set_on_message_received(application->server, application_on_message_received);

  return 0;

error0:
  entity_registry_fini(&application->entity_registry);
  block_registry_fini(&application->block_registry);
  return -1;
}

void application_fini(struct application *application)
{
  entity_manager_fini(&application->entity_manager);

  chunk_generator_fini(&application->chunk_generator);
  chunk_manager_fini(&application->chunk_manager);

  libnet_server_destroy(application->server);
  entity_registry_fini(&application->entity_registry);
  block_registry_fini(&application->block_registry);
}

void application_run(struct application *application)
{
  libnet_server_run(application->server);
}

void application_on_update(libnet_server_t server)
{
  struct application *application = libnet_server_get_opaque(server);

  chunk_manager_reset_active_chunks(&application->chunk_manager);

  for(entity_handle_t handle=0; handle<application->entity_manager.entities.item_count; ++handle)
  {
    struct entity *entity = &application->entity_manager.entities.items[handle];
    struct entity_info *info = &application->entity_registry.infos.items[entity->id];
    if(info->update)
      info->update(entity, &application->chunk_manager);
  }

  player_manager_update(FIXED_DT, application->server, &application->entity_manager);
  chunk_manager_update(&application->chunk_manager, &application->chunk_generator, application->server);
  entity_manager_update(&application->entity_manager, application->server);
}

void application_on_client_connected(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct application *application = libnet_server_get_opaque(server);

  player_manager_on_client_connected(server, client_proxy, &application->entity_manager);
  chunk_manager_on_client_connected(&application->chunk_manager, server, client_proxy);
  entity_manager_on_client_connected(&application->entity_manager, server, client_proxy);
}

void application_on_client_disconnected(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct application *application = libnet_server_get_opaque(server);

  player_manager_on_client_disconnected(server, client_proxy, &application->entity_manager);
}

void application_on_message_received(libnet_server_t server, libnet_client_proxy_t client_proxy, const struct libnet_message *message)
{
  player_manager_on_message_received(server, client_proxy, message);
}

