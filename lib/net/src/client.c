#include <libnet/client.h>

#include "socket.h"

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <libcore/ds/ring_buffer.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/eventfd.h>
#include <sys/queue.h>

#include <poll.h>
#include <netdb.h>
#include <unistd.h>

#include <pthread.h>

#include <stdio.h>
#include <errno.h>

enum libnet_client_request_type
{
  LIBNET_CLIENT_REQUEST_MESSAGE_SENT,
};

struct libnet_client_request
{
  enum libnet_client_request_type type;
  struct libnet_message *message;
};

static int efd;

static pthread_mutex_t request_mutex;
static struct libnet_client_request *request_queue;

static pthread_mutex_t event_mutex;
static struct libnet_client_event *event_queue;

static pthread_t thread;

struct param
{
  const char *node;
  const char *service;
};

static SSL_CTX *create_ssl_ctx(void)
{
  SSL_CTX *ssl_ctx = SSL_CTX_new(TLS_client_method());
  if(!ssl_ctx)
    exit(EXIT_FAILURE);

  SSL_CTX_clear_options(ssl_ctx, SSL_OP_NO_COMPRESSION);
  SSL_CTX_set_max_proto_version(ssl_ctx, TLS1_2_VERSION);
  SSL_CTX_set_security_level(ssl_ctx, 1);
  return ssl_ctx;
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
    exit(EXIT_FAILURE);
  }

  for(struct addrinfo *p = res; p; p = p->ai_next)
  {
    int fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if(fd == -1)
    {
      fprintf(stderr, "libnet: Warn: Failed to create socket: %s\n", strerror(errno));
      continue;
    }

    if(connect(fd, p->ai_addr, p->ai_addrlen) == -1)
    {
      fprintf(stderr, "libnet: Warn: Failed to connect socket: %s\n", strerror(errno));
      close(fd);
      continue;
    }

    freeaddrinfo(res);
    return fd;
  }

  fprintf(stderr, "libnet: Error: Failed to create any socket that connects to %s\n", service);
  exit(EXIT_FAILURE);
}


static void event_server_disconnected(void)
{
  struct libnet_client_event event = {0};
  event.type = LIBNET_CLIENT_EVENT_SERVER_DISCONNECTED;

  pthread_mutex_lock(&event_mutex);
  rb_push(event_queue, event);
  pthread_mutex_unlock(&event_mutex);
}

static void event_message_received(struct libnet_message *message)
{
  struct libnet_client_event event = {0};
  event.type = LIBNET_CLIENT_EVENT_MESSAGE_RECEIVED;
  event.message = message;

  pthread_mutex_lock(&event_mutex);
  rb_push(event_queue, event);
  pthread_mutex_unlock(&event_mutex);
}

static void notify_request(void)
{
  uint64_t value = 1;
  ssize_t n = TEMP_FAILURE_RETRY(write(efd, &value, sizeof value));
  if(n == -1)
  {
    fprintf(stderr, "libnet: Error: Failed to write(2) to eventfd(2): %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  if(n != sizeof value)
  {
    fprintf(stderr, "libnet: Error: Expected to write(2) %zu bytes to eventfd(2). Got %zu bytes.\n", sizeof value, n);
    exit(EXIT_FAILURE);
  }
}

static void request_message_sent(struct libnet_message *message)
{
  struct libnet_client_request request = {0};
  request.type = LIBNET_CLIENT_REQUEST_MESSAGE_SENT;
  request.message = message;

  pthread_mutex_lock(&request_mutex);
  rb_push(request_queue, request);
  pthread_mutex_unlock(&request_mutex);
  notify_request();
}

static void *handler(void *data)
{
  struct param *param = data;

  const char *node = param->node;
  const char *service = param->service;

  SSL_CTX *ssl_ctx = create_ssl_ctx();
  int socket_fd = socket_connect(node, service);

  struct ssl_socket socket;
  if(ssl_socket_connect(&socket, socket_fd, ssl_ctx) == -1)
  {
    fprintf(stderr, "libnet: Error: Failed to establish ssl connection with server: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  struct pollfd pollfds[2];
  pollfds[0].fd = efd;
  pollfds[0].events = POLLIN;
  pollfds[1].fd = socket_fd;

  for(;;)
  {
    pollfds[1].events = POLLIN;

    char *p;

    if(BIO_get_mem_data(socket.mwbio, &p))
      pollfds[1].events |= POLLOUT;

    if(BIO_get_mem_data(socket.wbio, &p))
      pollfds[1].events |= POLLOUT;

    if(TEMP_FAILURE_RETRY(poll(pollfds, sizeof pollfds / sizeof pollfds[0], -1)) == -1)
    {
      fprintf(stderr, "libnet: poll(2): %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    if(pollfds[0].revents & POLLIN)
    {
      uint64_t value;
      ssize_t n = TEMP_FAILURE_RETRY(read(efd, &value, sizeof value));
      if(n == -1)
      {
        fprintf(stderr, "libnet: Error: Failed to read(2) from eventfd(2): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }

      if(n != sizeof value)
      {
        fprintf(stderr, "libnet: Error: Expected to read(2) %zu bytes from eventfd(2). Got %zu bytes.\n", sizeof value, n);
        exit(EXIT_FAILURE);
      }

      assert(value == 1);

      pthread_mutex_lock(&request_mutex);

      struct libnet_client_request request = rb_pop(request_queue);
      switch(request.type)
      {
      case LIBNET_CLIENT_REQUEST_MESSAGE_SENT:
        ssl_socket_enqueue_message(&socket, request.message);
        free(request.message);
        break;
      }

      pthread_mutex_unlock(&request_mutex);
    }

    if(pollfds[1].revents & POLLIN)
    {
      ssize_t n = ssl_socket_recv(&socket);
      if(n < 0)
      {
        fprintf(stderr, "libnet: Error: failed to recv(2) from server: %s\n", strerror(errno));
        break;
      }

      if(n == 0)
        break;

      if(ssl_socket_try_decrypt(&socket) != 0)
        break;

      struct ssl_socket_message_iter iter = ssl_socket_message_iter_begin(&socket);
      struct libnet_message *message;
      while((message = ssl_socket_message_iter_next(&iter)))
        event_message_received(message);
    }

    if(pollfds[1].revents & POLLOUT)
    {
      if(ssl_socket_try_encrypt(&socket) != 0)
        break;

      ssize_t n = ssl_socket_send(&socket);
      if(n < 0)
      {
        fprintf(stderr, "libnet: Error: failed to send(2) to server: %s\n", strerror(errno));
        break;
      }
    }
  }

  event_server_disconnected();
  return NULL;
}

void libnet_client_run(const char *node, const char *service)
{
  efd = eventfd(0, EFD_SEMAPHORE);
  if(efd == -1)
  {
    fprintf(stderr, "libnet: Error: Failed to create eventfd: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  struct param *param = malloc(sizeof *param);
  param->node = node;
  param->service = service;

  int err = pthread_create(&thread, NULL, handler, param);
  if(err != 0)
  {
    fprintf(stderr, "libnet: Error: Failed to create background thread: %s\n", strerror(err));
    exit(EXIT_FAILURE);
  }
}

bool libnet_client_poll_event(struct libnet_client_event *event)
{
  pthread_mutex_lock(&event_mutex);

  bool result = rb_tail(event_queue) < rb_head(event_queue);
  if(result)
    *event = rb_pop(event_queue);

  pthread_mutex_unlock(&event_mutex);
  return result;
}

void libnet_client_send_message(struct libnet_message *message)
{
  request_message_sent(message);
}

