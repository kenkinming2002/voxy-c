#ifndef VOXY_SERVER_CONTEXT_H
#define VOXY_SERVER_CONTEXT_H

#include "chunk/block/manager.h"

#include "chunk/entity/database.h"

#include "player/manager.h"

#include "light/manager.h"

/// Context.
///
/// This bundles together all the data that may be accessed by a mod.
struct voxy_context
{
  libnet_server_t server;

  struct voxy_entity_database *entity_database;

  struct voxy_player_manager *player_manager;

  struct voxy_light_manager *light_manager;
};

#endif // VOXY_SERVER_CONTEXT_H
