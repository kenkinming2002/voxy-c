#include "application.h"
#include "config.h"

#include <stdio.h>

int application_init(struct application *application, int argc, char *argv[])
{
  if(argc != 2)
  {
    fprintf(stderr, "Usage: %s SERVICE", argv[0]);
    return -1;
  }

  block_registry_init(&application->block_registry);

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


  if(!(application->server = libnet_server_create(argv[1], FIXED_DT * 1e9)))
    goto error0;

  if(chunk_manager_init(&application->chunk_manager) != 0)
    goto error1;

  libnet_server_set_opaque(application->server, application);
  libnet_server_set_on_update(application->server, application_on_update);
  libnet_server_set_on_client_connected(application->server, application_on_client_connected);
  libnet_server_set_on_client_disconnected(application->server, application_on_client_disconnected);
  libnet_server_set_on_message_received(application->server, application_on_message_received);

  return 0;

error1:
  libnet_server_destroy(application->server);
error0:
  block_registry_fini(&application->block_registry);
  return -1;
}

void application_fini(struct application *application)
{
  chunk_manager_fini(&application->chunk_manager);
  libnet_server_destroy(application->server);
  block_registry_fini(&application->block_registry);
}

void application_run(struct application *application)
{
  libnet_server_run(application->server);
}

void application_on_update(libnet_server_t server)
{
  struct application *application = libnet_server_get_opaque(server);
  player_manager_update(application->server, FIXED_DT);
  chunk_manager_update(&application->chunk_manager);
}

void application_on_client_connected(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct application *application = libnet_server_get_opaque(server);

  player_manager_on_client_connected(server, client_proxy);
  chunk_manager_on_client_connected(&application->chunk_manager, server, client_proxy);
}

void application_on_client_disconnected(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  player_manager_on_client_disconnected(server, client_proxy);
}

void application_on_message_received(libnet_server_t server, libnet_client_proxy_t client_proxy, const struct libnet_message *message)
{
  player_manager_on_message_received(server, client_proxy, message);
}

