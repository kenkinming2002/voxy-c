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

#include <libcore/time.h>
#include <libcore/profile.h>

#include <stb_ds.h>

#include <stdlib.h>
#include <stdio.h>

void on_update(void)
{
  profile_scope;

  voxy_reset_chunk_regions();

  struct voxy_entity *entities = entity_get_all();
  for(entity_handle_t handle=0; handle<arrlenu(entities); ++handle)
  {
    struct voxy_entity *entity = &entities[handle];
    if(!entity->alive)
      continue;

    struct voxy_entity_info info = voxy_query_entity(entity->id);
    if(info.update && !info.update(entity, FIXED_DT))
      voxy_entity_despawn(handle);
  }

  physics_update(FIXED_DT);
  light_update();

  voxy_block_database_update();
  voxy_block_manager_update();
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
  voxy_player_manager_on_message_received(client_proxy, message);
}

static void init(int argc, char *argv[])
{
  if(argc < 5)
  {
    fprintf(stderr, "Usage: %s SERVICE CERT KEY WORLD_DIRECTORY [MOD]...", argv[0]);
    exit(EXIT_FAILURE);
  }

  libnet_server_run(argv[1], argv[2], argv[3]);

  voxy_block_database_init(argv[4]);
  voxy_block_generator_init(argv[4]);
  voxy_entity_database_init(argv[4]);

  for(int i=5; i<argc; ++i)
    mod_load(argv[i]);

  voxy_entity_manager_start();
}

int main(int argc, char *argv[])
{
  init(argc, argv);

  double compensation = 0.0f;
  for(;;)
  {
    const double begin = time_get();

    on_update();

    struct libnet_server_event event;
    while(libnet_server_poll_event(&event))
      switch(event.type)
      {
      case LIBNET_SERVER_EVENT_CLIENT_CONNECTED:
        libnet_server_ack_client_connected(event.client_proxy);
        on_client_connected(event.client_proxy);
        break;
      case LIBNET_SERVER_EVENT_CLIENT_DISCONNECTED:
        libnet_server_ack_client_disconnected(event.client_proxy);
        on_client_disconnected(event.client_proxy);
        break;
      case LIBNET_SERVER_EVENT_MESSAGE_RECEIVED:
        on_message_received(event.client_proxy, event.message);
        free(event.message);
        break;
      }

    const double end = time_get();
    const double diff = end - begin + compensation;
    if(diff > FIXED_DT)
    {
      // Compensation makes occasional frame stutter feel not as bad, because
      // subsequent updates need not get delayed. (They are still noticeable
      // though)
      //
      // However, we do not allow unbounded compensation. Otherwise, you will
      // become sonic the hedgehog after a lag spike.
      compensation = diff - FIXED_DT;
      if(compensation > 1 * FIXED_DT)
        compensation = 1 * FIXED_DT;
    }
    else
    {
      compensation = 0.0;
      time_sleep(FIXED_DT - diff);
    }
  }
}

