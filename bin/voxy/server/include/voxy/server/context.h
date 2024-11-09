#ifndef VOXY_SERVER_CONTEXT_H
#define VOXY_SERVER_CONTEXT_H

#include "block/registry.h"
#include "entity/registry.h"

#include "chunk/manager.h"
#include "chunk/generator.h"

#include "entity/manager.h"
#include "entity/database.h"
#include "player/manager.h"

#include "light/manager.h"

/// Context.
///
/// This bundles together all the data that may be accessed by a mod.
struct voxy_context
{
  libnet_server_t server;

  struct voxy_block_registry *block_registry;
  struct voxy_entity_registry *entity_registry;

  struct voxy_chunk_manager *chunk_manager;
  struct voxy_chunk_generator *chunk_generator;

  struct voxy_entity_manager *entity_manager;
  struct voxy_entity_database *entity_database;
  struct voxy_player_manager *player_manager;

  struct voxy_light_manager *light_manager;
};

#endif // VOXY_SERVER_CONTEXT_H
