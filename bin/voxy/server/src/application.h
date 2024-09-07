#ifndef APPLICATION_H
#define APPLICATION_H

#include <libnet/server.h>

#include "block/registry.h"
#include "entity/registry.h"

#include "chunk/manager.h"
#include "chunk/generator.h"

#include "entity/manager.h"
#include "player/manager.h"

#include "light/manager.h"

#include "mod/manager.h"

struct application
{
  struct voxy_block_registry block_registry;
  struct voxy_entity_registry entity_registry;

  libnet_server_t server;

  struct voxy_chunk_manager chunk_manager;
  struct chunk_generator chunk_generator;

  struct voxy_entity_manager entity_manager;
  struct voxy_player_manager player_manager;

  struct light_manager light_manager;

  struct mod_manager mod_manager;
};

/// Initialize/finalize application.
int application_init(struct application *application, int argc, char *argv[]);
void application_fini(struct application *application);

/// Create context from application.
struct voxy_context application_get_context(struct application *application);

/// Run the application.
void application_run(struct application *application);

/// Callbacks.
void application_on_update(libnet_server_t server);
void application_on_client_connected(libnet_server_t server, libnet_client_proxy_t client_proxy);
void application_on_client_disconnected(libnet_server_t server, libnet_client_proxy_t client_proxy);
void application_on_message_received(libnet_server_t server, libnet_client_proxy_t client_proxy, const struct libnet_message *message);

#endif // APPLICATION_H
