#ifndef APPLICATION_H
#define APPLICATION_H

#include <libnet/client.h>

struct application
{
};

/// Initialize/finalize application.
int application_init(struct application *application, int argc, char *argv[]);
void application_fini(struct application *application);

/// Create context from application.
struct voxy_context application_get_context(struct application *application);

/// Run the application.
void application_run(struct application *application);

/// Network callback.
void application_on_message_received(const struct libnet_message *message);

#endif // APPLICATION_H
