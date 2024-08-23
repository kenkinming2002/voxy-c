#include <libnet/client.h>

#include "socket.h"

#include <fcntl.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct libnet_client
{
  struct socket socket;
  libnet_client_on_message_received_t on_message_received;
};

libnet_client_t libnet_client_create(const char *node, const char *service)
{
  struct addrinfo hints = {0};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *res;
  int result;
  if((result = getaddrinfo(node, service, &hints, &res)) != 0)
  {
    fprintf(stderr, "libnet: Error: Failed to create client: %s\n", gai_strerror(result));
    return NULL;
  }

  for(struct addrinfo *p = res; p; p = p->ai_next)
  {
    int fd;
    if((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
      continue;

    if(connect(fd, p->ai_addr, p->ai_addrlen) == -1)
    {
      fprintf(stderr, "libnet: Warn: Failed to connect socket: %s\n", strerror(errno));
      close(fd);
      continue;
    }

    freeaddrinfo(res);

    libnet_client_t client = malloc(sizeof *client);
    client->socket = socket_create_from_fd(fd);
    client->on_message_received = NULL;
    return client;
  }

  fprintf(stderr, "libnet: Error: Failed to create any socket\n");
  freeaddrinfo(res);
  return NULL;
}

void libnet_client_destroy(libnet_client_t client)
{
  socket_destroy(client->socket);
  free(client);
}

void libnet_client_set_on_message_received(libnet_client_t client, libnet_client_on_message_received_t cb)
{
  client->on_message_received = cb;
}


int libnet_client_update(libnet_client_t client)
{
  int connection_closed = 0;

  if(socket_update_try_send(&client->socket) != 0)
    fprintf(stderr, "libnet: Warn: Failed to send message: %s\n", strerror(errno));

  if(socket_update_try_recv(&client->socket, &connection_closed) != 0)
    fprintf(stderr, "libnet: Warn: Failed to recv message: %s\n", strerror(errno));

  const struct libnet_message *message;

  size_t i = 0;
  while((message = socket_dequeue_message(&client->socket, &i)))
    if(client->on_message_received)
      client->on_message_received(client, message);

  socket_dequeue_message_end(&client->socket, i);

  return connection_closed;
}

void libnet_client_send_message(struct libnet_client *client, struct libnet_message *message)
{
  socket_enqueue_message(&client->socket, message);
}

