#include "application.h"
#include "config.h"

#include "physics/physics.h"

#include <voxy/server/context.h>

#include <libcommon/core/log.h>
#include <libcommon/core/profile.h>

#include <stdio.h>
#include <time.h>

int application_init(struct application *application, int argc, char *argv[])
{
  if(argc != 2)
  {
    fprintf(stderr, "Usage: %s SERVICE", argv[0]);
    return -1;
  }

  voxy_block_registry_init(&application->block_registry);
  voxy_entity_registry_init(&application->entity_registry);
  voxy_item_registry_init(&application->item_registry);

  if(!(application->server = libnet_server_create(argv[1], FIXED_DT * 1e9)))
    goto error0;

  libnet_server_set_opaque(application->server, application);
  libnet_server_set_on_update(application->server, application_on_update);
  libnet_server_set_on_client_connected(application->server, application_on_client_connected);
  libnet_server_set_on_client_disconnected(application->server, application_on_client_disconnected);
  libnet_server_set_on_message_received(application->server, application_on_message_received);

  voxy_chunk_manager_init(&application->chunk_manager);
  voxy_chunk_generator_init(&application->chunk_generator, time(NULL));

  voxy_entity_manager_init(&application->entity_manager);
  if(voxy_entity_database_init(&application->entity_database) != 0) goto error1;
  voxy_player_manager_init(&application->player_manager);

  voxy_light_manager_init(&application->light_manager);

  struct voxy_context context = application_get_context(application);
  mod_manager_init(&application->mod_manager);
  if(mod_manager_load(&application->mod_manager, "bin/mod/base/server/base-server.so", &context) != 0)
    goto error2;

  return 0;

error2:
  voxy_light_manager_fini(&application->light_manager);
  voxy_player_manager_fini(&application->player_manager);
error1:
  voxy_entity_database_fini(&application->entity_database);
  voxy_entity_manager_fini(&application->entity_manager);
  voxy_chunk_generator_fini(&application->chunk_generator);
  voxy_chunk_manager_fini(&application->chunk_manager);
  libnet_server_destroy(application->server);
error0:
  voxy_item_registry_fini(&application->item_registry);
  voxy_entity_registry_fini(&application->entity_registry);
  voxy_block_registry_fini(&application->block_registry);
  return -1;
}

void application_fini(struct application *application)
{
  struct voxy_context context = application_get_context(application);
  mod_manager_fini(&application->mod_manager, &context);

  voxy_light_manager_fini(&application->light_manager);

  voxy_player_manager_fini(&application->player_manager);
  voxy_entity_database_fini(&application->entity_database);
  voxy_entity_manager_fini(&application->entity_manager);

  voxy_chunk_generator_fini(&application->chunk_generator);
  voxy_chunk_manager_fini(&application->chunk_manager);

  libnet_server_destroy(application->server);

  voxy_item_registry_fini(&application->item_registry);
  voxy_entity_registry_fini(&application->entity_registry);
  voxy_block_registry_fini(&application->block_registry);
}

struct voxy_context application_get_context(struct application *application)
{
  struct voxy_context context;
  context.server = application->server;
  context.block_registry = &application->block_registry;
  context.entity_registry = &application->entity_registry;
  context.item_registry = &application->item_registry;
  context.chunk_manager = &application->chunk_manager;
  context.chunk_generator = &application->chunk_generator;
  context.entity_manager = &application->entity_manager;
  context.entity_database = &application->entity_database;
  context.player_manager = &application->player_manager;
  context.light_manager = &application->light_manager;
  return context;
}

void application_run(struct application *application)
{
  voxy_entity_manager_start(&application->entity_manager, &application->entity_registry, &application->entity_database, application->server);
  libnet_server_run(application->server);
}

void application_on_update(libnet_server_t server)
{
  profile_begin();

  struct application *application = libnet_server_get_opaque(server);

  const struct voxy_context context = application_get_context(application);

  voxy_chunk_manager_reset_active_chunks(&application->chunk_manager);

  for(entity_handle_t handle=0; handle<application->entity_manager.allocator.entities.item_count; ++handle)
  {
    struct voxy_entity *entity = &application->entity_manager.allocator.entities.items[handle];
    if(!entity->alive)
      continue;

    struct voxy_entity_info info = voxy_entity_registry_query_entity(&application->entity_registry, entity->id);
    if(info.update && !info.update(entity, FIXED_DT, &context))
      voxy_entity_manager_despawn(&application->entity_manager, &application->entity_database, handle, server);
  }

  physics_update(&application->block_registry, &application->entity_registry, &application->chunk_manager, &application->entity_manager, FIXED_DT);
  voxy_light_manager_update(&application->light_manager, &application->block_registry, &application->chunk_manager);

  voxy_chunk_manager_update(&application->chunk_manager, &application->chunk_generator, &application->light_manager, application->server, &context);
  voxy_entity_manager_update(&application->entity_manager, &application->entity_registry, &application->entity_database, &application->chunk_manager, application->server);

  profile_end();
}

void application_on_client_connected(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct application *application = libnet_server_get_opaque(server);
  const struct voxy_context context = application_get_context(application);

  voxy_chunk_manager_on_client_connected(&application->chunk_manager, server, client_proxy);
  voxy_entity_manager_on_client_connected(&application->entity_manager, server, client_proxy);
  voxy_player_manager_on_client_connected(&application->player_manager, server, client_proxy, &context);
}

void application_on_client_disconnected(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct application *application = libnet_server_get_opaque(server);

  voxy_player_manager_on_client_disconnected(&application->player_manager, server, client_proxy);
}

void application_on_message_received(libnet_server_t server, libnet_client_proxy_t client_proxy, const struct libnet_message *message)
{
  struct application *application = libnet_server_get_opaque(server);

  voxy_player_manager_on_message_received(&application->player_manager, server, client_proxy, message);
}

