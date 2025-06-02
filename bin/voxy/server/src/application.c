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

  if(!(application->server = libnet_server_create(argv[1], argv[2], argv[3], FIXED_DT * 1e9)))
    goto error0;

  libnet_server_set_opaque(application->server, application);
  libnet_server_set_on_update(application->server, application_on_update);
  libnet_server_set_on_client_connected(application->server, application_on_client_connected);
  libnet_server_set_on_client_disconnected(application->server, application_on_client_disconnected);
  libnet_server_set_on_message_received(application->server, application_on_message_received);

  voxy_block_database_init(argv[4]);
  voxy_block_generator_init(argv[4]);

  voxy_entity_database_init(argv[4]);

  mod_manager_init(&application->mod_manager);

  struct voxy_context context = application_get_context(application);
  for(int i=5; i<argc; ++i)
    if(mod_manager_load(&application->mod_manager, argv[i], &context) != 0)
      goto error2;

  return 0;

error2:
  mod_manager_fini(&application->mod_manager, &context);

  libnet_server_destroy(application->server);
error0:
  return -1;
}

void application_fini(struct application *application)
{
  struct voxy_context context = application_get_context(application);
  mod_manager_fini(&application->mod_manager, &context);
  libnet_server_destroy(application->server);
}

struct voxy_context application_get_context(struct application *application)
{
  struct voxy_context context;

  context.server = application->server;

  return context;
}

void application_run(struct application *application)
{
  voxy_entity_manager_start(application->server);
  libnet_server_run(application->server);
}

void application_on_update(libnet_server_t server)
{
  profile_scope;

  struct application *application = libnet_server_get_opaque(server);

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
      voxy_entity_despawn(handle, server);
  }

  physics_update(FIXED_DT);
  light_update();

  voxy_block_database_update();
  voxy_block_manager_update(application->server, &context);
  voxy_entity_manager_update(application->server);
}

void application_on_client_connected(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  voxy_block_manager_on_client_connected(server, client_proxy);
  voxy_entity_manager_on_client_connected(server, client_proxy);
  voxy_player_manager_on_client_connected(server, client_proxy);
}

void application_on_client_disconnected(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  voxy_player_manager_on_client_disconnected(server, client_proxy);
}

void application_on_message_received(libnet_server_t server, libnet_client_proxy_t client_proxy, const struct libnet_message *message)
{
  struct application *application = libnet_server_get_opaque(server);
  const struct voxy_context context = application_get_context(application);
  voxy_player_manager_on_message_received(server, client_proxy, message, &context);
}

