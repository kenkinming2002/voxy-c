#ifndef LIBNET_CLIENT_H
#define LIBNET_CLIENT_H

#include "message.h"

#include <stdbool.h>

/// Type of events for server.
enum libnet_client_event_type
{
  LIBNET_CLIENT_EVENT_SERVER_DISCONNECTED,
  LIBNET_CLIENT_EVENT_MESSAGE_RECEIVED,
};

/// Event for client
struct libnet_client_event
{
  enum libnet_client_event_type type;
  struct libnet_message *message;
};

/// Run the client.
///
/// The provided node and service name has the same semantic as the parameter to
/// getaddrinfo(3) for socket with type SOCK_STREAM.
///
/// For example, node can be a domain name or ip address and service can be
/// either a numeric string specifyin the port number, or name for one of the
/// services founded in /etc/services on linux system.
///
/// This will start handling connections and messages from clients and putting
/// events into the event queue.
///
/// This simply call exit(3) on error because why would you need otherwise?
void libnet_client_run(const char *node, const char *service);

/// Poll for next event from the event queue.
bool libnet_client_poll_event(struct libnet_client_event *event);

/// Send message to server.
void libnet_client_send_message(struct libnet_message *message);

#endif // LIBNET_CLIENT_H
