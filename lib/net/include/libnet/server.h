#ifndef LIBNET_SERVER_H
#define LIBNET_SERVER_H

#include "message.h"

#include <stdbool.h>

/// A client proxy.
///
/// This represent a client from the server point of view.
typedef struct libnet_client_proxy *libnet_client_proxy_t;

/// Type of events for server.
enum libnet_server_event_type
{
  LIBNET_SERVER_EVENT_CLIENT_CONNECTED,
  LIBNET_SERVER_EVENT_CLIENT_DISCONNECTED,
  LIBNET_SERVER_EVENT_MESSAGE_RECEIVED,
};

/// Event for server.
struct libnet_server_event
{
  enum libnet_server_event_type type;
  struct libnet_client_proxy *client_proxy;
  struct libnet_message *message;
};

/// Set/get opaque pointer on client proxy object.
void libnet_client_proxy_set_opaque(libnet_client_proxy_t client_proxy, void *opaque);
void *libnet_client_proxy_get_opaque(libnet_client_proxy_t client_proxy);

/// Run the server.
///
/// The provided service name has the same semantic as the parameter to
/// getaddrinfo(3) for socket with type SOCK_STREAM. For example, it can be
/// either a numeric string specifyin the port number, or name for one of the
/// services founded in /etc/services on linux system.
///
/// The certficiate parameter is path to the server certificate used for TLS
/// connection.
///
/// This will start handling connections and messages from clients and putting
/// events into the event queue.
///
/// This simply call exit(3) on error because why would you need otherwise?
void libnet_server_run(const char *service, const char *cert, const char *key);

/// Poll for next event from the event queue.
bool libnet_server_poll_event(struct libnet_server_event *event);

/// Acknowledge that a client has been connected.
///
/// This is necessary for subsequent calls to libnet_server_send_message(NULL,
/// ...) to act on the new client.
void libnet_server_ack_client_connected(libnet_client_proxy_t client_proxy);

/// Acknowledge that a client has been disconnected.
///
/// This tells the server that the client can be deleted.
void libnet_server_ack_client_disconnected(libnet_client_proxy_t client_proxy);

/// Send a message to the client through given client_proxy.
///
/// If client_proxy is NULL, this is a broadcast. This takes ownership of
/// message which is assumed to be dynamically allocated with malloc(3).
///
/// TODO: Determine if this should block/Determine a buffering strategy.
void libnet_server_send_message(libnet_client_proxy_t client_proxy, struct libnet_message *message);

#endif // LIBNET_SERVER_H
