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

static struct ssl_socket ssl_socket;

static void *opaque;
static libnet_client_on_message_received_t on_message_received;

static SSL_CTX *create_ssl_ctx(void)
{
  SSL_CTX *ssl_ctx = SSL_CTX_new(TLS_client_method());
  if(!ssl_ctx)
    goto err;

  SSL_CTX_clear_options(ssl_ctx, SSL_OP_NO_COMPRESSION);
  SSL_CTX_set_max_proto_version(ssl_ctx, TLS1_2_VERSION);
  SSL_CTX_set_security_level(ssl_ctx, 1);
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

void libnet_client_init(const char *node, const char *service)
{
  SSL_CTX *ssl_ctx = create_ssl_ctx();
  if(!ssl_ctx)
    exit(EXIT_FAILURE);

  int fd;
  if((fd = socket_connect(node, service)) == -1)
    exit(EXIT_FAILURE);

  if(ssl_socket_connect(&ssl_socket, fd, ssl_ctx) != 0)
    exit(EXIT_FAILURE);

  SSL_CTX_free(ssl_ctx);
}

void libnet_client_deinit(void)
{
  ssl_socket_destroy(&ssl_socket);
}

void libnet_client_set_opaque(void *_opaque)
{
  opaque = _opaque;
}

void *libnet_client_get_opaque(void)
{
  return opaque;
}

void libnet_client_set_on_message_received(libnet_client_on_message_received_t cb)
{
  on_message_received = cb;
}

bool libnet_client_update(void)
{
  bool connection_closed = false;

  if(ssl_socket_try_recv(&ssl_socket, &connection_closed) != 0)
    return true;

  if(ssl_socket_try_decrypt(&ssl_socket) != 0)
    return true;

  struct ssl_socket_message_iter iter = ssl_socket_message_iter_begin(&ssl_socket);
  const struct libnet_message *message;
  while((message = ssl_socket_message_iter_next(&iter)))
    if(on_message_received)
      on_message_received(message);

  if(ssl_socket_try_encrypt(&ssl_socket) != 0)
    return true;

  if(ssl_socket_try_send(&ssl_socket) != 0)
    return true;

  return connection_closed;
}

void libnet_client_send_message(struct libnet_message *message)
{
  ssl_socket_enqueue_message(&ssl_socket, message);
}

