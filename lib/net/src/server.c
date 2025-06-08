#include <libnet/server.h>

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

#include <stb_ds.h>
#include <pthread.h>

#include <stdio.h>
#include <errno.h>

enum libnet_server_request_type
{
  LIBNET_SERVER_REQUEST_ACK_CLIENT_CONNECTED,
  LIBNET_SERVER_REQUEST_ACK_CLIENT_DISCONNECTED,
  LIBNET_SERVER_REQUEST_MESSAGE_SENT,
};

struct libnet_server_request
{
  enum libnet_server_request_type type;
  struct libnet_client_proxy *client_proxy;
  struct libnet_message *message;
};

struct libnet_client_proxy
{
  LIST_ENTRY(libnet_client_proxy) entry;

  struct ssl_socket socket;
  void *opaque;
};

LIST_HEAD(libnet_client_proxy_list, libnet_client_proxy);

void libnet_client_proxy_set_opaque(libnet_client_proxy_t client_proxy, void *opaque)
{
  client_proxy->opaque = opaque;
}

void *libnet_client_proxy_get_opaque(libnet_client_proxy_t client_proxy)
{
  return client_proxy->opaque;
}

static int efd;

static pthread_mutex_t request_mutex;
static struct libnet_server_request *request_queue;

static pthread_mutex_t event_mutex;
static struct libnet_server_event *event_queue;

static pthread_t thread;

struct param
{
  const char *service;
  const char *cert;
  const char *key;
};

static int socket_bind(const char *service)
{
  struct addrinfo hints = {0};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *res;
  int result;
  if((result = getaddrinfo(NULL, service, &hints, &res)) != 0)
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

    int yes = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    {
      fprintf(stderr, "libnet: Warn: Failed to set socket option: %s\n", strerror(errno));
      close(fd);
      continue;
    }

    if(bind(fd, p->ai_addr, p->ai_addrlen) == -1)
    {
      fprintf(stderr, "libnet: Warn: Failed to bind socket: %s\n", strerror(errno));
      close(fd);
      continue;
    }

    if(listen(fd, 32) == -1)
    {
      fprintf(stderr, "libnet: Warn: Failed to set socket to listening mode: %s\n", strerror(errno));
      close(fd);
      continue;
    }

    freeaddrinfo(res);
    return fd;
  }

  fprintf(stderr, "libnet: Error: Failed to create any socket that listens on %s\n", service);
  exit(EXIT_FAILURE);
}

static SSL_CTX *create_ssl_ctx(const char *cert, const char *key)
{
  SSL_CTX *ssl_ctx = SSL_CTX_new(TLS_server_method());
  if(!ssl_ctx)
    exit(EXIT_FAILURE);

  if(SSL_CTX_use_certificate_file(ssl_ctx, cert, SSL_FILETYPE_PEM) <= 0)
  {
    fprintf(stderr, "libnet: Error: Failed to load SSL certificate from %s: %s\n", cert, ERR_error_string(ERR_get_error(), NULL));
    exit(EXIT_FAILURE);
  }

  if(SSL_CTX_use_PrivateKey_file(ssl_ctx, key, SSL_FILETYPE_PEM) <= 0)
  {
    fprintf(stderr, "libnet: Error: Failed to load SSL private key from %s: %s\n", key, ERR_error_string(ERR_get_error(), NULL));
    exit(EXIT_FAILURE);
  }

  if(!SSL_CTX_check_private_key(ssl_ctx))
  {
    fprintf(stderr, "libnet: Error: Failed to check SSL private key: %s\n", ERR_error_string(ERR_get_error(), NULL));
    exit(EXIT_FAILURE);
  }

  SSL_CTX_set_max_proto_version(ssl_ctx, TLS1_2_VERSION);
  SSL_CTX_set_security_level(ssl_ctx, 1);
  SSL_CTX_clear_options(ssl_ctx, SSL_OP_NO_COMPRESSION);

  return ssl_ctx;
}

static void event_client_connected(libnet_client_proxy_t client_proxy)
{
  struct libnet_server_event event = {0};
  event.type = LIBNET_SERVER_EVENT_CLIENT_CONNECTED;
  event.client_proxy = client_proxy;

  pthread_mutex_lock(&event_mutex);
  rb_push(event_queue, event);
  pthread_mutex_unlock(&event_mutex);
}

static void event_client_disconnected(libnet_client_proxy_t client_proxy)
{
  struct libnet_server_event event = {0};
  event.type = LIBNET_SERVER_EVENT_CLIENT_DISCONNECTED;
  event.client_proxy = client_proxy;

  pthread_mutex_lock(&event_mutex);
  rb_push(event_queue, event);
  pthread_mutex_unlock(&event_mutex);
}

static void event_message_received(libnet_client_proxy_t client_proxy, struct libnet_message *message)
{
  struct libnet_server_event event = {0};
  event.type = LIBNET_SERVER_EVENT_MESSAGE_RECEIVED;
  event.client_proxy = client_proxy;
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

static void request_ack_client_connect(libnet_client_proxy_t client_proxy)
{
  struct libnet_server_request request = {0};
  request.type = LIBNET_SERVER_REQUEST_ACK_CLIENT_CONNECTED;
  request.client_proxy = client_proxy;

  pthread_mutex_lock(&request_mutex);
  rb_push(request_queue, request);
  pthread_mutex_unlock(&request_mutex);
  notify_request();
}

static void request_ack_client_disconnect(libnet_client_proxy_t client_proxy)
{
  struct libnet_server_request request = {0};
  request.type = LIBNET_SERVER_REQUEST_ACK_CLIENT_DISCONNECTED;
  request.client_proxy = client_proxy;

  pthread_mutex_lock(&request_mutex);
  rb_push(request_queue, request);
  pthread_mutex_unlock(&request_mutex);
  notify_request();
}

static void request_message_sent(libnet_client_proxy_t client_proxy, struct libnet_message *message)
{
  struct libnet_server_request request = {0};
  request.type = LIBNET_SERVER_REQUEST_MESSAGE_SENT;
  request.client_proxy = client_proxy;
  request.message = message;

  pthread_mutex_lock(&request_mutex);
  rb_push(request_queue, request);
  pthread_mutex_unlock(&request_mutex);
  notify_request();
}

static void *handler(void *data)
{
  struct param *param = data;

  const char *service = param->service;
  const char *cert = param->cert;
  const char *key = param->key;

  SSL_CTX *ssl_ctx = create_ssl_ctx(cert, key);
  int server_socket = socket_bind(service);

  struct libnet_client_proxy_list active_client_proxies = LIST_HEAD_INITIALIZER(active_client_proxies);
  struct libnet_client_proxy_list inactive_client_proxies = LIST_HEAD_INITIALIZER(inactive_client_proxies);

  struct pollfd *pollfds = NULL;
  struct libnet_client_proxy **polled_client_proxies = NULL;

  for(;;)
  {
    struct pollfd pollfd = {0};

    arrsetlen(pollfds, 0);
    arrsetlen(polled_client_proxies, 0);

    pollfd.fd = efd;
    pollfd.events = POLLIN;
    arrput(pollfds, pollfd);

    pollfd.fd = server_socket;
    pollfd.events = POLLIN;
    arrput(pollfds, pollfd);

    struct libnet_client_proxy *active_client_proxy;
    LIST_FOREACH(active_client_proxy, &active_client_proxies, entry)
    {
      pollfd.fd = active_client_proxy->socket.fd;
      pollfd.events = POLLIN;

      char *p;

      if(BIO_get_mem_data(active_client_proxy->socket.mwbio, &p))
        pollfd.events |= POLLOUT;

      if(BIO_get_mem_data(active_client_proxy->socket.wbio, &p))
        pollfd.events |= POLLOUT;

      arrput(pollfds, pollfd);
      arrput(polled_client_proxies, active_client_proxy);
    }

    if(TEMP_FAILURE_RETRY(poll(pollfds, arrlen(pollfds), -1)) == -1)
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

      struct libnet_server_request request = rb_pop(request_queue);
      switch(request.type)
      {
      case LIBNET_SERVER_REQUEST_ACK_CLIENT_CONNECTED:
        // inactive -> active
        LIST_REMOVE(request.client_proxy, entry);
        LIST_INSERT_HEAD(&active_client_proxies, request.client_proxy, entry);
        break;
      case LIBNET_SERVER_REQUEST_ACK_CLIENT_DISCONNECTED:
        // inactive -> none
        LIST_REMOVE(request.client_proxy, entry);
        ssl_socket_destroy(&request.client_proxy->socket);
        free(request.client_proxy);
        break;
      case LIBNET_SERVER_REQUEST_MESSAGE_SENT:
        if(request.client_proxy)
          ssl_socket_enqueue_message(&request.client_proxy->socket, request.message);
        else
          LIST_FOREACH(active_client_proxy, &active_client_proxies, entry)
            ssl_socket_enqueue_message(&active_client_proxy->socket, request.message);

        free(request.message);
        break;
      }

      pthread_mutex_unlock(&request_mutex);
    }

    if(pollfds[1].revents & POLLIN)
    {
      struct sockaddr_storage address;
      socklen_t address_len = sizeof address;

      int client_socket = accept(server_socket, (struct sockaddr *)&address, &address_len);
      if(client_socket != -1)
      {
        struct libnet_client_proxy *inactive_client_proxy = malloc(sizeof *inactive_client_proxy);
        if(ssl_socket_accept(&inactive_client_proxy->socket, client_socket, ssl_ctx) == 0)
        {
          // none -> inactive
          LIST_INSERT_HEAD(&inactive_client_proxies, inactive_client_proxy, entry);
          event_client_connected(inactive_client_proxy);
        }
        else
          fprintf(stderr, "libnet: Warn: Failed to establish ssl connection with client: %s\n", strerror(errno));
      }
      else
        fprintf(stderr, "libnet: Warn: Failed to accept connection from client: %s\n", strerror(errno));
    }

    for(size_t i=2; i<arrlenu(pollfds); ++i)
    {
      struct libnet_client_proxy *client_proxy = polled_client_proxies[i-2];

      if(pollfds[i].revents & POLLIN)
      {
        ssize_t n = ssl_socket_recv(&client_proxy->socket);
        if(n <= 0)
          goto err;

        if(ssl_socket_try_decrypt(&client_proxy->socket) != 0)
          goto err;

        struct ssl_socket_message_iter iter = ssl_socket_message_iter_begin(&client_proxy->socket);
        struct libnet_message *message;
        while((message = ssl_socket_message_iter_next(&iter)))
          event_message_received(client_proxy, message);
      }

      if(pollfds[i].revents & POLLOUT)
      {
        if(ssl_socket_try_encrypt(&client_proxy->socket) != 0)
          goto err;

        ssize_t n = ssl_socket_send(&client_proxy->socket);
        if(n < 0)
          goto err;
      }

      continue;

err:
      // active -> inactive
      LIST_REMOVE(client_proxy, entry);
      LIST_INSERT_HEAD(&inactive_client_proxies, client_proxy, entry);
      event_client_disconnected(client_proxy);
    }
  }

  return NULL;

}

void libnet_server_run(const char *service, const char *cert, const char *key)
{
  efd = eventfd(0, EFD_SEMAPHORE);
  if(efd == -1)
  {
    fprintf(stderr, "libnet: Error: Failed to create eventfd: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  struct param *param = malloc(sizeof *param);
  param->service = service;
  param->cert = cert;
  param->key = key;

  int err = pthread_create(&thread, NULL, handler, param);
  if(err != 0)
  {
    fprintf(stderr, "libnet: Error: Failed to create background thread: %s\n", strerror(err));
    exit(EXIT_FAILURE);
  }
}

bool libnet_server_poll_event(struct libnet_server_event *event)
{
  pthread_mutex_lock(&event_mutex);

  bool result = rb_tail(event_queue) < rb_head(event_queue);
  if(result)
    *event = rb_pop(event_queue);

  pthread_mutex_unlock(&event_mutex);
  return result;
}

void libnet_server_ack_client_connected(libnet_client_proxy_t client_proxy)
{
  request_ack_client_connect(client_proxy);
}

void libnet_server_ack_client_disconnected(libnet_client_proxy_t client_proxy)
{
  request_ack_client_disconnect(client_proxy);
}

void libnet_server_send_message(libnet_client_proxy_t client_proxy, struct libnet_message *message)
{
  request_message_sent(client_proxy, message);
}

