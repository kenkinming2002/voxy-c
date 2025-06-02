#include "application.h"
#include "config.h"

#include "chunk/manager.h"
#include "chunk/block/manager.h"
#include "chunk/block/database.h"
#include "chunk/block/generator.h"
#include "chunk/entity/allocator.h"
#include "chunk/entity/manager.h"
#include "chunk/entity/database.h"

#include "light/light.h"
#include "physics/physics.h"

#include "player/manager.h"

#include "mod/mod.h"

#include <voxy/server/context.h>

#include <libcore/log.h>
#include <libcore/profile.h>

#include <stb_ds.h>

#include <stdio.h>

int application_init(struct application *application, int argc, char *argv[])
{
  if(argc < 5)
  {
    fprintf(stderr, "Usage: %s SERVICE CERT KEY WORLD_DIRECTORY [MOD]...", argv[0]);
    return -1;
  }

  libnet_server_init(argv[1], argv[2], argv[3], FIXED_DT * 1e9);

  libnet_server_set_opaque(application);
  libnet_server_set_on_update(application_on_update);
  libnet_server_set_on_client_connected(application_on_client_connected);
  libnet_server_set_on_client_disconnected(application_on_client_disconnected);
  libnet_server_set_on_message_received(application_on_message_received);

  voxy_block_database_init(argv[4]);
  voxy_block_generator_init(argv[4]);
  voxy_entity_database_init(argv[4]);

  struct voxy_context context = application_get_context(application);
  for(int i=5; i<argc; ++i)
    mod_load(argv[i], &context);

  return 0;
}

void application_fini(struct application *application)
{
  libnet_server_deinit();
}

struct voxy_context application_get_context(struct application *application)
{
  struct voxy_context context;
  return context;
}

void application_run(struct application *application)
{
  voxy_entity_manager_start();
  libnet_server_run();
}

void application_on_update(void)
{
  profile_scope;

  struct application *application = libnet_server_get_opaque();

  const struct voxy_context context = application_get_context(application);

  voxy_reset_active_chunks();

  struct voxy_entity *entities = entity_get_all();
  for(entity_handle_t handle=0; handle<arrlenu(entities); ++handle)
  {
    struct voxy_entity *entity = &entities[handle];
    if(!entity->alive)
      continue;

    struct voxy_entity_info info = voxy_query_entity(entity->id);
    if(info.update && !info.update(entity, FIXED_DT, &context))
      voxy_entity_despawn(handle);
  }

  physics_update(FIXED_DT);
  light_update();

  voxy_block_database_update();
  voxy_block_manager_update(&context);
  voxy_entity_manager_update();
}

void application_on_client_connected(libnet_client_proxy_t client_proxy)
{
  voxy_block_manager_on_client_connected(client_proxy);
  voxy_entity_manager_on_client_connected(client_proxy);
  voxy_player_manager_on_client_connected(client_proxy);
}

void application_on_client_disconnected(libnet_client_proxy_t client_proxy)
{
  voxy_player_manager_on_client_disconnected(client_proxy);
}

void application_on_message_received(libnet_client_proxy_t client_proxy, const struct libnet_message *message)
{
  struct application *application = libnet_server_get_opaque();
  const struct voxy_context context = application_get_context(application);
  voxy_player_manager_on_message_received(client_proxy, message, &context);
}

