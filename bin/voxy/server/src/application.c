#include "application.h"
#include "config.h"

#include <voxy/server/context.h>

#include <libcommon/core/log.h>

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

  if(!(application->server = libnet_server_create(argv[1], FIXED_DT * 1e9)))
    goto error0;

  libnet_server_set_opaque(application->server, application);
  libnet_server_set_on_update(application->server, application_on_update);
  libnet_server_set_on_client_connected(application->server, application_on_client_connected);
  libnet_server_set_on_client_disconnected(application->server, application_on_client_disconnected);
  libnet_server_set_on_message_received(application->server, application_on_message_received);

  chunk_manager_init(&application->chunk_manager);
  chunk_generator_init(&application->chunk_generator, time(NULL));

  voxy_entity_manager_init(&application->entity_manager);

  struct voxy_context context = application_get_context(application);
  mod_manager_init(&application->mod_manager);
  if(mod_manager_load(&application->mod_manager, "bin/mod/base/server/base-server.so", &context) != 0)
    goto error1;

  return 0;

error1:
  entity_manager_fini(&application->entity_manager);
  chunk_generator_fini(&application->chunk_generator);
  chunk_manager_fini(&application->chunk_manager);
  libnet_server_destroy(application->server);
error0:
  voxy_entity_registry_fini(&application->entity_registry);
  voxy_block_registry_fini(&application->block_registry);
  return -1;
}

void application_fini(struct application *application)
{
  struct voxy_context context = application_get_context(application);
  mod_manager_fini(&application->mod_manager, &context);

  entity_manager_fini(&application->entity_manager);

  chunk_generator_fini(&application->chunk_generator);
  chunk_manager_fini(&application->chunk_manager);

  libnet_server_destroy(application->server);

  voxy_entity_registry_fini(&application->entity_registry);
  voxy_block_registry_fini(&application->block_registry);
}

struct voxy_context application_get_context(struct application *application)
{
  struct voxy_context context;
  context.server = application->server;
  context.block_registry = &application->block_registry;
  context.entity_registry = &application->entity_registry;
  context.chunk_manager = &application->chunk_manager;
  context.entity_manager = &application->entity_manager;
  return context;
}

void application_run(struct application *application)
{
  libnet_server_run(application->server);
}

void application_on_update(libnet_server_t server)
{
  struct application *application = libnet_server_get_opaque(server);

  chunk_manager_reset_active_chunks(&application->chunk_manager);

  const struct voxy_context context = application_get_context(application);
  for(entity_handle_t handle=0; handle<application->entity_manager.entities.item_count; ++handle)
  {
    struct voxy_entity *entity = &application->entity_manager.entities.items[handle];
    struct voxy_entity_info *info = &application->entity_registry.infos.items[entity->id];
    if(info->update)
      info->update(entity, &context);
  }

  player_manager_update(FIXED_DT, application->server, &application->entity_manager);
  chunk_manager_update(&application->chunk_manager, &application->chunk_generator, application->server);
  voxy_entity_manager_update(&application->entity_manager, application->server);
}

void application_on_client_connected(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct application *application = libnet_server_get_opaque(server);

  player_manager_on_client_connected(server, client_proxy, &application->entity_manager);
  chunk_manager_on_client_connected(&application->chunk_manager, server, client_proxy);
  voxy_entity_manager_on_client_connected(&application->entity_manager, server, client_proxy);
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

