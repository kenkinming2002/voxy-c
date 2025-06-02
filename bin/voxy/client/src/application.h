#ifndef APPLICATION_H
#define APPLICATION_H

#include <libnet/client.h>

#include "render/world.h"

struct application
{
  libnet_client_t client;
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
