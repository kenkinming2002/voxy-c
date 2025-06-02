#ifndef LIBNET_CLIENT_H
#define LIBNET_CLIENT_H

#include "message.h"

#include <stdbool.h>

/// Initialize libnet client.
///
/// This indicates that you wish to use libnet in client mode.
///
/// The provided node and service name has the same semantic as the parameter to
/// getaddrinfo(3) for socket with type SOCK_STREAM.
///
/// For example, node can be a domain name or ip address and service can be
/// either a numeric string specifyin the port number, or name for one of the
/// services founded in /etc/services on linux system.
void libnet_client_init(const char *node, const char *service);

/// Deinitialize libnet client.
///
/// This is supposed to close down the connection cleanly but currently just
/// reset the connection by calling close(2) on the socket.
void libnet_client_deinit(void);

/// Handle all pending network events.
///
/// Return non-zero value if connection was closed.
bool libnet_client_update(void);

/// Event callbacks.
typedef void(*libnet_client_on_message_received_t)(const struct libnet_message *message);

/// Opaque pointer.
void libnet_client_set_opaque(void *opaque);
void *libnet_client_get_opaque(void);

/// Event callbacks setters.
void libnet_client_set_on_message_received(libnet_client_on_message_received_t cb);

/// Send a message to the server.
///
/// This will buffer the message internally to be sent during the next call to
/// libnet_client_update().
void libnet_client_send_message(struct libnet_message *message);

#endif // LIBNET_CLIENT_H
