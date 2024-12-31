#ifndef LIBNET_CLIENT_H
#define LIBNET_CLIENT_H

#include "message.h"

#include <stdbool.h>

/// Client type.
typedef struct libnet_client *libnet_client_t;

/// Create a libnet client.
///
/// The provided node and service name has the same semantic as the parameter to
/// getaddrinfo(3) for socket with type SOCK_STREAM.
///
/// For example, node can be a domain name or ip address and service can be
/// either a numeric string specifyin the port number, or name for one of the
/// services founded in /etc/services on linux system.
libnet_client_t libnet_client_create(const char *node, const char *service);

/// Destroy a libnet client.
void libnet_client_destroy(libnet_client_t client);

/// Event callbacks.
typedef void(*libnet_client_on_message_received_t)(libnet_client_t client, const struct libnet_message *message);

/// Opaque pointer.
void libnet_client_set_opaque(libnet_client_t client, void *opaque);
void *libnet_client_get_opaque(libnet_client_t client);

/// Event callbacks setters.
void libnet_client_set_on_message_received(libnet_client_t client, libnet_client_on_message_received_t cb);

/// Handle all pending network events.
///
/// Return non-zero value if connection was closed.
bool libnet_client_update(libnet_client_t client);

/// Send a message to the server.
///
/// This will buffer the message internally to be sent during the next call to
/// libnet_client_update().
void libnet_client_send_message(struct libnet_client *client, struct libnet_message *message);

#endif // LIBNET_CLIENT_H
