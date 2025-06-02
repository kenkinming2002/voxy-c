#include "config.h"

#include "chunk/manager.h"

#include "chunk/block/database.h"
#include "chunk/block/generator.h"
#include "chunk/block/manager.h"

#include "chunk/entity/allocator.h"
#include "chunk/entity/database.h"
#include "chunk/entity/manager.h"

#include "light/light.h"
#include "physics/physics.h"

#include "player/manager.h"

#include "mod/mod.h"

#include <libcore/profile.h>

#include <stdlib.h>
#include <stdio.h>

void on_update(void)
{
  profile_scope;

  voxy_reset_active_chunks();

  struct voxy_entity *entities = entity_get_all();
  for(entity_handle_t handle=0; handle<arrlenu(entities); ++handle)
  {
    struct voxy_entity *entity = &entities[handle];
    if(!entity->alive)
      continue;

    struct voxy_entity_info info = voxy_query_entity(entity->id);
    if(info.update && !info.update(entity, FIXED_DT, NULL))
      voxy_entity_despawn(handle);
  }

  physics_update(FIXED_DT);
  light_update();

  voxy_block_database_update();
  voxy_block_manager_update(NULL);
  voxy_entity_manager_update();
}

void on_client_connected(libnet_client_proxy_t client_proxy)
{
  voxy_block_manager_on_client_connected(client_proxy);
  voxy_entity_manager_on_client_connected(client_proxy);
  voxy_player_manager_on_client_connected(client_proxy);
}

void on_client_disconnected(libnet_client_proxy_t client_proxy)
{
  voxy_player_manager_on_client_disconnected(client_proxy);
}

void on_message_received(libnet_client_proxy_t client_proxy, const struct libnet_message *message)
{
  voxy_player_manager_on_message_received(client_proxy, message, NULL);
}

static void init(int argc, char *argv[])
{
  if(argc < 5)
  {
    fprintf(stderr, "Usage: %s SERVICE CERT KEY WORLD_DIRECTORY [MOD]...", argv[0]);
    exit(EXIT_FAILURE);
  }

  libnet_server_init(argv[1], argv[2], argv[3], FIXED_DT * 1e9);

  libnet_server_set_on_update(on_update);
  libnet_server_set_on_client_connected(on_client_connected);
  libnet_server_set_on_client_disconnected(on_client_disconnected);
  libnet_server_set_on_message_received(on_message_received);

  voxy_block_database_init(argv[4]);
  voxy_block_generator_init(argv[4]);
  voxy_entity_database_init(argv[4]);

  for(int i=5; i<argc; ++i)
    mod_load(argv[i], NULL);
}

static void deinit(void)
{
  libnet_server_deinit();
}

int main(int argc, char *argv[])
{
  init(argc, argv);
  libnet_server_run();
  deinit();
}

