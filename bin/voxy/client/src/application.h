#ifndef APPLICATION_H
#define APPLICATION_H

#include <libnet/client.h>

#include "input/manager.h"
#include "camera/manager.h"

#include "chunk/block/manager.h"
#include "chunk/entity/manager.h"

#include "render/world.h"

#include "mod/manager.h"

struct application
{
  libnet_client_t client;

  struct input_manager input_manager;
  struct camera_manager camera_manager;

  struct block_manager block_manager;
  struct entity_manager entity_manager;

  struct mod_manager mod_manager;

  struct world_renderer world_renderer;
};

/// Initialize/finalize application.
int application_init(struct application *application, int argc, char *argv[]);
void application_fini(struct application *application);

/// Create context from application.
struct voxy_context application_get_context(struct application *application);

/// Run the application.
void application_run(struct application *application);

/// Network callback.
void application_on_message_received(libnet_client_t client, const struct libnet_message *message);

#endif // APPLICATION_H
