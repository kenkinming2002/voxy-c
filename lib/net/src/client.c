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
  struct ssl_socket socket;

  void *opaque;
  libnet_client_on_message_received_t on_message_received;
};

static SSL_CTX *create_ssl_ctx(void)
{
  SSL_CTX *ssl_ctx = SSL_CTX_new(TLS_client_method());
  if(!ssl_ctx)
    goto err;

  return ssl_ctx;

err:
  return NULL;
}

static void set_nonblock(int fd)
{
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

static int socket_connect_impl(struct addrinfo *p)
{
  int fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
  if(fd == -1)
  {
    fprintf(stderr, "libnet: Warn: Failed to create socket: %s\n", strerror(errno));
    return -1;
  }

  if(connect(fd, p->ai_addr, p->ai_addrlen) == -1)
  {
    fprintf(stderr, "libnet: Warn: Failed to connect socket: %s\n", strerror(errno));
    goto err_close_fd;
  }

  set_nonblock(fd);
  return fd;

err_close_fd:
  close(fd);
  return -1;
}

static int socket_connect(const char *node, const char *service)
{
  struct addrinfo hints = {0};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *res;
  int result;
  if((result = getaddrinfo(node, service, &hints, &res)) != 0)
  {
    fprintf(stderr, "libnet: Error: Failed to resolve %s: %s\n", service, gai_strerror(result));
    return -1;
  }

  int fd = -1;
  for(struct addrinfo *p = res; p; p = p->ai_next)
    if((fd = socket_connect_impl(p)) != -1)
      break;

  freeaddrinfo(res);
  return fd;
}

libnet_client_t libnet_client_create(const char *node, const char *service)
{
  libnet_client_t client = malloc(sizeof *client);

  SSL_CTX *ssl_ctx = create_ssl_ctx();
  if(!ssl_ctx)
    goto err_free_client;

  int fd;
  if((fd = socket_connect(node, service)) == -1)
    goto err_free_ssl_ctx;

  if(ssl_socket_connect(&client->socket, fd, ssl_ctx) != 0)
    goto err_close_fd;

  SSL_CTX_free(ssl_ctx);

  client->opaque = NULL;
  client->on_message_received = NULL;
  return client;

err_close_fd:
  close(fd);
err_free_ssl_ctx:
  SSL_CTX_free(ssl_ctx);
err_free_client:
  free(client);
  return NULL;
}

void libnet_client_destroy(libnet_client_t client)
{
  ssl_socket_destroy(&client->socket);
  free(client);
}

void libnet_client_set_opaque(libnet_client_t client, void *opaque)
{
  client->opaque = opaque;
}

void *libnet_client_get_opaque(libnet_client_t client)
{
  return client->opaque;
}

void libnet_client_set_on_message_received(libnet_client_t client, libnet_client_on_message_received_t cb)
{
  client->on_message_received = cb;
}

bool libnet_client_update(libnet_client_t client)
{
  bool connection_closed = false;

  if(ssl_socket_try_recv(&client->socket, &connection_closed) != 0)
    return true;

  if(ssl_socket_try_decrypt(&client->socket) != 0)
    return true;

  struct ssl_socket_message_iter iter = ssl_socket_message_iter_begin(&client->socket);
  const struct libnet_message *message;
  while((message = ssl_socket_message_iter_next(&iter)))
    if(client->on_message_received)
      client->on_message_received(client, message);

  if(ssl_socket_try_encrypt(&client->socket) != 0)
    return true;

  if(ssl_socket_try_send(&client->socket) != 0)
    return true;

  return connection_closed;
}

void libnet_client_send_message(struct libnet_client *client, struct libnet_message *message)
{
  ssl_socket_enqueue_message(&client->socket, message);
}

