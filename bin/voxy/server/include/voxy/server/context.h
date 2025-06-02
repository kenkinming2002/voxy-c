#ifndef VOXY_SERVER_CONTEXT_H
#define VOXY_SERVER_CONTEXT_H

#include <libnet/server.h>

/// Context.
///
/// This bundles together all the data that may be accessed by a mod.
struct voxy_context
{
  libnet_server_t server;
};

#endif // VOXY_SERVER_CONTEXT_H
