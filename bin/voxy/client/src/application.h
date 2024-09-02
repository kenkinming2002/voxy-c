#ifndef APPLICATION_H
#define APPLICATION_H

#include <libnet/client.h>

#include "block/registry.h"
#include "entity/registry.h"

#include "input/manager.h"
#include "camera/manager.h"

#include "chunk/manager.h"
#include "entity/manager.h"

#include "render/world.h"

struct application
{
  struct block_registry block_registry;
  struct entity_registry entity_registry;

  libnet_client_t client;

  struct input_manager input_manager;
  struct camera_manager camera_manager;

  struct chunk_manager chunk_manager;
  struct entity_manager entity_manager;

  struct world_renderer world_renderer;
};

/// Initialize/finalize application.
int application_init(struct application *application, int argc, char *argv[]);
void application_fini(struct application *application);

/// Run the application.
void application_run(struct application *application);

/// Network callback.
void application_on_message_received(libnet_client_t client, const struct libnet_message *message);

#endif // APPLICATION_H
