#include <libnet/server.h>

#include "socket.h"

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <fcntl.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/timerfd.h>
#include <sys/signalfd.h>
#include <unistd.h>
#include <signal.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct libnet_client_proxy
{
  LIST_ENTRY(libnet_client_proxy) entry;

  struct ssl_socket socket;
  void *opaque;
};

LIST_HEAD(libnet_client_proxies, libnet_client_proxy);

static libnet_client_proxy_t client_proxy_create(int fd, SSL_CTX *ssl_ctx)
{
  libnet_client_proxy_t client_proxy = malloc(sizeof *client_proxy);
  if(ssl_socket_accept(&client_proxy->socket, fd, ssl_ctx) != 0)
  {
    free(client_proxy);
    return NULL;
  }
  client_proxy->opaque = NULL;
  return client_proxy;
}

static void client_proxy_destroy(libnet_client_proxy_t client_proxy)
{
  ssl_socket_destroy(&client_proxy->socket);
  free(client_proxy);
}

void libnet_client_proxy_set_opaque(libnet_client_proxy_t client_proxy, void *opaque)
{
  client_proxy->opaque = opaque;
}

void *libnet_client_proxy_get_opaque(libnet_client_proxy_t client_proxy)
{
  return client_proxy->opaque;
}

struct libnet_server
{
  SSL_CTX *ssl_ctx;

  int socket;
  int timerfd;
  int signalfd;

  int epollfd;

  struct libnet_client_proxies client_proxies;

  void *opaque;
  libnet_server_on_update on_update;
  libnet_server_on_client_connected_t on_client_connected;
  libnet_server_on_client_disconnected_t on_client_disconnected;
  libnet_server_on_message_received_t on_message_received;
};

static SSL_CTX *create_ssl_ctx(const char *cert, const char *key)
{
  SSL_CTX *ssl_ctx = SSL_CTX_new(TLS_server_method());
  if(!ssl_ctx)
    goto err;

  if(SSL_CTX_use_certificate_file(ssl_ctx, cert, SSL_FILETYPE_PEM) <= 0)
  {
    fprintf(stderr, "libnet: Error: Failed to load SSL certificate from %s: %s\n", cert, ERR_error_string(ERR_get_error(), NULL));
    goto err_free_ctx;
  }

  if(SSL_CTX_use_PrivateKey_file(ssl_ctx, key, SSL_FILETYPE_PEM) <= 0)
  {
    fprintf(stderr, "libnet: Error: Failed to load SSL private key from %s: %s\n", key, ERR_error_string(ERR_get_error(), NULL));
    goto err_free_ctx;
  }

  if(!SSL_CTX_check_private_key(ssl_ctx))
  {
    fprintf(stderr, "libnet: Error: Failed to check SSL private key: %s\n", ERR_error_string(ERR_get_error(), NULL));
    goto err_free_ctx;
  }

  SSL_CTX_set_max_proto_version(ssl_ctx, TLS1_2_VERSION);
  SSL_CTX_set_security_level(ssl_ctx, 1);
  SSL_CTX_clear_options(ssl_ctx, SSL_OP_NO_COMPRESSION);
  return ssl_ctx;

err_free_ctx:
  SSL_CTX_free(ssl_ctx);
err:
  return NULL;
}

static void set_nonblock(int fd)
{
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

static int socket_bind_impl(struct addrinfo *p)
{
  int fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
  if(fd == -1)
  {
    fprintf(stderr, "libnet: Warn: Failed to create socket: %s\n", strerror(errno));
    return -1;
  }

  if(bind(fd, p->ai_addr, p->ai_addrlen) == -1)
  {
    fprintf(stderr, "libnet: Warn: Failed to bind socket: %s\n", strerror(errno));
    goto err_close_fd;
  }

  if(listen(fd, 4) == -1)
  {
    fprintf(stderr, "libnet: Warn: Failed to set socket to listening mode: %s\n", strerror(errno));
    goto err_close_fd;
  }

  set_nonblock(fd);
  return fd;

err_close_fd:
  close(fd);
  return -1;
}

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
    return -1;
  }

  int fd = -1;
  for(struct addrinfo *p = res; p; p = p->ai_next)
    if((fd = socket_bind_impl(p)) != -1)
      break;

  freeaddrinfo(res);
  return fd;
}

static int create_timerfd(unsigned long long nsec)
{
  int fd = fd = timerfd_create(CLOCK_MONOTONIC, 0);
  if(fd == -1)
  {
    fprintf(stderr, "libnet: Error: Failed to create timer file descriptor: %s\n", strerror(errno));
    return -1;
  }

  struct itimerspec itimerspec;
  itimerspec.it_value.tv_sec = 0;
  itimerspec.it_value.tv_nsec = nsec;
  itimerspec.it_interval.tv_sec = 0;
  itimerspec.it_interval.tv_nsec = nsec;
  if(timerfd_settime(fd, 0, &itimerspec, NULL) == -1)
  {
    fprintf(stderr, "libnet: Error: Failed to configure timer file descriptor: %s\n", strerror(errno));
    close(fd);
    return -1;
  }

  set_nonblock(fd);
  return fd;
}

static int create_signalfd(void)
{
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);

  int fd = signalfd(-1, &mask, SFD_NONBLOCK);
  if(fd == -1)
  {
    fprintf(stderr, "libnet: Error: Failed to create signal file descriptor: %s\n", strerror(errno));
    return -1;
  }

  return fd;
}

static int create_epollfd(void)
{
  int fd = epoll_create1(0);
  if(fd == -1)
  {
    fprintf(stderr, "libnet: Error: Failed to create epoll file descriptor: %s\n", strerror(errno));
    return -1;
  }
  return fd;
}

static int configure_epollfd_in(int fd, int *rfd)
{
  struct epoll_event epoll_event;
  epoll_event.events = EPOLLIN;
  epoll_event.data.ptr = rfd;
  if(epoll_ctl(fd, EPOLL_CTL_ADD, *rfd, &epoll_event) == -1)
  {
    fprintf(stderr, "libnet: Error: Failed to configure epoll file descriptor: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}

libnet_server_t libnet_server_create(const char *service, const char *cert, const char *key, unsigned long long nsec)
{
  libnet_server_t server = malloc(sizeof *server);

  if(!(server->ssl_ctx = create_ssl_ctx(cert, key)))
    goto err_free_server;

  if((server->socket = socket_bind(service)) == -1)
    goto err_free_ssl_ctx;

  if((server->timerfd = create_timerfd(nsec)) == -1)
    goto err_close_socket;

  if((server->signalfd = create_signalfd()) == -1)
    goto err_close_timerfd;

  if((server->epollfd = create_epollfd()) == -1)
    goto err_close_signalfd;

  if(configure_epollfd_in(server->epollfd, &server->socket) != 0)
    goto err_close_epollfd;

  if(configure_epollfd_in(server->epollfd, &server->timerfd) != 0)
    goto err_close_epollfd;

  if(configure_epollfd_in(server->epollfd, &server->signalfd) != 0)
    goto err_close_epollfd;

  LIST_INIT(&server->client_proxies);
  server->opaque = NULL;
  server->on_client_connected = NULL;
  server->on_client_disconnected = NULL;
  server->on_message_received = NULL;
  return server;

err_close_epollfd:
  close(server->epollfd);
err_close_signalfd:
  close(server->signalfd);
err_close_timerfd:
  close(server->timerfd);
err_close_socket:
  close(server->socket);
err_free_ssl_ctx:
  SSL_CTX_free(server->ssl_ctx);
err_free_server:
  free(server);
  return NULL;
}

void libnet_server_destroy(libnet_server_t server)
{
  while(!LIST_EMPTY(&server->client_proxies))
  {
    libnet_client_proxy_t client_proxy = LIST_FIRST(&server->client_proxies);
    LIST_REMOVE(client_proxy, entry);

    ssl_socket_destroy(&client_proxy->socket);
    free(client_proxy);
  }

  close(server->epollfd);
  close(server->socket);
  close(server->timerfd);
  close(server->signalfd);
  free(server);
}

void libnet_server_set_opaque(libnet_server_t server, void *opaque)
{
  server->opaque = opaque;
}

void *libnet_server_get_opaque(libnet_server_t server)
{
  return server->opaque;
}

void libnet_server_set_on_update(libnet_server_t server, libnet_server_on_update cb)
{
  server->on_update = cb;
}

void libnet_server_set_on_client_connected(libnet_server_t server, libnet_server_on_client_connected_t cb)
{
  server->on_client_connected = cb;
}

void libnet_server_set_on_client_disconnected(libnet_server_t server, libnet_server_on_client_disconnected_t cb)
{
  server->on_client_disconnected = cb;
}

void libnet_server_set_on_message_received(libnet_server_t server, libnet_server_on_message_received_t cb)
{
  server->on_message_received = cb;
}

static int accept_client(int fd)
{
  struct sockaddr_storage address;
  socklen_t address_len = sizeof address;

  int client = accept(fd, (struct sockaddr *)&address, &address_len);
  if(client == -1)
  {
    if(errno != EAGAIN && errno != EWOULDBLOCK)
      fprintf(stderr, "libnet: Warn: Failed to accept connection: %s\n", strerror(errno));
    return -1;
  }

  set_nonblock(client);
  return client;
}

static int read_timerfd(int fd, uint64_t *count)
{
  ssize_t n = read(fd, count, sizeof *count);
  if(n == -1)
  {
    fprintf(stderr, "libnet: Warn: Failed to read from timerfd: %s\n", strerror(errno));
    return -1;
  }

  if(n != sizeof *count)
  {
    fprintf(stderr, "libnet: Warn: Expected %zu bytes from timerfd. Got %zu bytes.\n", sizeof *count, n);
    return -1;
  }

  return 0;
}

static int read_signalfd(int fd, struct signalfd_siginfo *siginfo)
{
  ssize_t n = read(fd, siginfo, sizeof *siginfo);
  if(n == -1)
  {
    fprintf(stderr, "libnet: Warn: Error from signalfd: %s\n", strerror(errno));
    return -1;
  }

  if(n != sizeof *siginfo)
  {
    fprintf(stderr, "libnet: Warn: Expected %zu bytes from signalfd. Got %zu bytes.\n", sizeof *siginfo, n);
    return -1;
  }

  return 0;
}

static void libnet_server_insert_client_proxy(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct epoll_event epoll_event;
  epoll_event.events = EPOLLIN;
  epoll_event.data.ptr = client_proxy;
  if(epoll_ctl(server->epollfd, EPOLL_CTL_ADD, client_proxy->socket.fd, &epoll_event))
  {
    fprintf(stderr, "libnet: Error: Failed to configure epoll file descriptor: %s\n", strerror(errno));
    abort();
  }

  LIST_INSERT_HEAD(&server->client_proxies, client_proxy, entry);
  if(server->on_client_connected) server->on_client_connected(server, client_proxy);
}

static void libnet_server_remove_client_proxy(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  if(server->on_client_disconnected) server->on_client_disconnected(server, client_proxy);
  LIST_REMOVE(client_proxy, entry);

  struct epoll_event epoll_event;
  if(epoll_ctl(server->epollfd, EPOLL_CTL_DEL, client_proxy->socket.fd, &epoll_event) != 0)
  {
    fprintf(stderr, "libnet: Error: Failed to configure epoll file descriptor: %s\n", strerror(errno));
    abort();
  }
}

void libnet_server_run(libnet_server_t server)
{
  sigset_t mask;
  sigset_t oldmask;

  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  pthread_sigmask(SIG_BLOCK, &mask, &oldmask);

  for(;;)
  {
    struct epoll_event epoll_events[32];
    int epoll_event_count = epoll_wait(server->epollfd, epoll_events, sizeof epoll_events / sizeof epoll_events[0], -1);
    if(epoll_event_count == -1)
    {
      fprintf(stderr, "libnet: Warn: Failed to wait on epoll file descriptor: %s\n", strerror(errno));
      continue;
    }

    for(int i=0; i<epoll_event_count; ++i)
    {
      const struct epoll_event epoll_event = epoll_events[i];
      if(epoll_event.data.ptr == &server->socket)
      {
        int client_fd;
        while((client_fd = accept_client(server->socket)) != -1)
        {
          libnet_client_proxy_t client_proxy = client_proxy_create(client_fd, server->ssl_ctx);
          if(!client_proxy) continue;
          libnet_server_insert_client_proxy(server, client_proxy);
        }
      }
      else if(epoll_event.data.ptr == &server->timerfd)
      {
        uint64_t count;
        if(read_timerfd(server->timerfd, &count) != 0)
          continue;

        for(uint64_t i=0; i<count; ++i)
          server->on_update(server);
      }
      else if(epoll_event.data.ptr == &server->signalfd)
      {
        struct signalfd_siginfo siginfo;
        if(read_signalfd(server->signalfd, &siginfo) != 0)
          continue;

        fprintf(stderr, "libnet: Info: Caught %s... Exiting.\n", strsignal(siginfo.ssi_signo));
        goto out;
      }
      else
      {
        libnet_client_proxy_t client_proxy = epoll_event.data.ptr;

        const bool old_want_send = ssl_socket_want_send(&client_proxy->socket);
        {
          bool connection_closed = 0;
          if(epoll_event.events & EPOLLIN && ssl_socket_try_recv(&client_proxy->socket, &connection_closed) != 0)
          {
            libnet_server_remove_client_proxy(server, client_proxy);
            continue;
          }

          if(ssl_socket_try_decrypt(&client_proxy->socket) != 0)
          {
            libnet_server_remove_client_proxy(server, client_proxy);
            continue;
          }

          struct ssl_socket_message_iter iter = ssl_socket_message_iter_begin(&client_proxy->socket);
          const struct libnet_message *message;
          while((message = ssl_socket_message_iter_next(&iter)))
            if(server->on_message_received)
              server->on_message_received(server, client_proxy, message);

          if(connection_closed)
          {
            libnet_server_remove_client_proxy(server, client_proxy);
            client_proxy_destroy(client_proxy);
            continue;
          }

          if(ssl_socket_try_encrypt(&client_proxy->socket) != 0)
          {
            libnet_server_remove_client_proxy(server, client_proxy);
            client_proxy_destroy(client_proxy);
            continue;
          }

          if(epoll_event.events & EPOLLOUT && ssl_socket_try_send(&client_proxy->socket) != 0)
          {
            libnet_server_remove_client_proxy(server, client_proxy);
            client_proxy_destroy(client_proxy);
            continue;
          }
        }
        const bool new_want_send = ssl_socket_want_send(&client_proxy->socket);

        if(old_want_send != new_want_send)
        {
          struct epoll_event epoll_event;
          epoll_event.events = new_want_send ? EPOLLIN | EPOLLOUT : EPOLLIN;
          epoll_event.data.ptr = client_proxy;
          if(epoll_ctl(server->epollfd, EPOLL_CTL_MOD, client_proxy->socket.fd, &epoll_event) != 0)
          {
            fprintf(stderr, "libnet: Error: Failed to configure epoll file descriptor: %s\n", strerror(errno));
            abort();
          }
        }
      }
    }
  }

out:
  pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
}

void libnet_server_foreach_client(libnet_server_t server, void(*cb)(libnet_server_t server, libnet_client_proxy_t client, void *data), void *data)
{
  libnet_client_proxy_t client_proxy;
  LIST_FOREACH(client_proxy, &server->client_proxies, entry)
    cb(server, client_proxy, data);
}

void libnet_server_send_message_all(libnet_server_t server, const struct libnet_message *message)
{
  libnet_client_proxy_t client_proxy;
  LIST_FOREACH(client_proxy, &server->client_proxies, entry)
    libnet_server_send_message(server, client_proxy,  message);
}

void libnet_server_send_message(libnet_server_t server, libnet_client_proxy_t client_proxy, const struct libnet_message *message)
{
  // Ensure that we are polling for whether we can write on the underlying
  // socket.
  if(!ssl_socket_want_send(&client_proxy->socket))
  {
    struct epoll_event epoll_event;
    epoll_event.events = EPOLLIN | EPOLLOUT;
    epoll_event.data.ptr = client_proxy;
    if(epoll_ctl(server->epollfd, EPOLL_CTL_MOD, client_proxy->socket.fd, &epoll_event) != 0)
    {
      fprintf(stderr, "libnet: Error: Failed to configure epoll file descriptor: %s\n", strerror(errno));
      abort();
    }
  }
  ssl_socket_enqueue_message(&client_proxy->socket, message);
}

