#ifndef LIBNET_SERVER_H
#define LIBNET_SERVER_H

#include "message.h"

/// A client proxy.
///
/// This represent a client from the server point of view.
typedef struct libnet_client_proxy *libnet_client_proxy_t;

/// Server type.
typedef struct libnet_server *libnet_server_t;

/// Create a libnet server.
///
/// The provided service name has the same semantic as the parameter to
/// getaddrinfo(3) for socket with type SOCK_STREAM. For example, it can be
/// either a numeric string specifyin the port number, or name for one of the
/// services founded in /etc/services on linux system.
libnet_server_t libnet_server_create(const char *service);

/// Destroy a libnet server.
void libnet_server_destroy(libnet_server_t server);

/// Event callbacks.
typedef void(*libnet_server_on_client_connected_t)(libnet_server_t server, libnet_client_proxy_t client_proxy);
typedef void(*libnet_server_on_client_disconnected_t)(libnet_server_t server, libnet_client_proxy_t client_proxy);
typedef void(*libnet_server_on_message_received_t)(libnet_server_t server, libnet_client_proxy_t client_proxy, const struct libnet_message *message);

/// Event callbacks setters.
void libnet_server_set_on_client_connected(libnet_server_t server, libnet_server_on_client_connected_t cb);
void libnet_server_set_on_client_disconnected(libnet_server_t server, libnet_server_on_client_disconnected_t cb);
void libnet_server_set_on_message_received(libnet_server_t server, libnet_server_on_message_received_t cb);

/// Handle all pending network events.
void libnet_server_update(libnet_server_t server);

/// Send a message to all connected clients.
///
/// TODO: Determine if this should block/Determine a buffering strategy.
void libnet_server_send_message_all(libnet_server_t server, const struct libnet_message *message);

/// Send a message to the given client.
///
/// TODO: Determine if this should block/Determine a buffering strategy.
void libnet_server_send_message(libnet_server_t server, libnet_client_proxy_t client_proxy, const struct libnet_message *message);

#endif // LIBNET_SERVER_H
