#ifndef APPLICATION_H
#define APPLICATION_H

#include <libnet/server.h>

#include "chunk/block/manager.h"
#include "chunk/block/database.h"

#include "chunk/entity/manager.h"
#include "chunk/entity/database.h"

#include "player/manager.h"

#include "light/manager.h"

#include "mod/manager.h"

struct application
{
  libnet_server_t server;

  struct voxy_block_manager block_manager;
  struct voxy_block_database block_database;

  struct voxy_entity_manager entity_manager;
  struct voxy_entity_database entity_database;

  struct voxy_player_manager player_manager;

  struct voxy_light_manager light_manager;

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
