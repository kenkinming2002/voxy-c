#include "application.h"

#include <stdio.h>

int application_init(struct application *application, int argc, char *argv[])
{
  if(argc != 2)
  {
    fprintf(stderr, "Usage: %s SERVICE", argv[0]);
    return -1;
  }

  if(!(application->server = libnet_server_create(argv[1])))
    return -1;

  if(chunk_manager_init(&application->chunk_manager) != 0)
  {
    libnet_server_destroy(application->server);
    return -1;
  }

  libnet_server_set_opaque(application->server, application);
  libnet_server_set_on_client_connected(application->server, application_on_client_connected);
  libnet_server_set_on_client_disconnected(application->server, application_on_client_disconnected);
  libnet_server_set_on_message_received(application->server, application_on_message_received);

  return 0;
}

void application_fini(struct application *application)
{
  chunk_manager_fini(&application->chunk_manager);
  libnet_server_destroy(application->server);
}

void application_run(struct application *application)
{
  for(;;)
  {
    libnet_server_update(application->server);
    chunk_manager_update(&application->chunk_manager);
  }
}

void application_on_client_connected(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct application *application = libnet_server_get_opaque(server);
  chunk_manager_on_client_connected(&application->chunk_manager, server, client_proxy);
}

void application_on_client_disconnected(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
}

void application_on_message_received(libnet_server_t server, libnet_client_proxy_t client_proxy, const struct libnet_message *message)
{
}

