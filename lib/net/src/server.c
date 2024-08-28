#include <libnet/server.h>

#include "socket.h"

#include <fcntl.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct libnet_client_proxy
{
  LIST_ENTRY(libnet_client_proxy) entry;
  struct socket socket;

  void *opaque;
};

LIST_HEAD(libnet_client_proxies, libnet_client_proxy);

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
  int epoll_fd;
  int socket_fd;
  int timer_fd;

  struct libnet_client_proxies client_proxies;

  void *opaque;
  libnet_server_on_update on_update;
  libnet_server_on_client_connected_t on_client_connected;
  libnet_server_on_client_disconnected_t on_client_disconnected;
  libnet_server_on_message_received_t on_message_received;
};

libnet_server_t libnet_server_create(const char *service, unsigned long long nsec)
{
  libnet_server_t server = malloc(sizeof *server);

  struct addrinfo hints = {0};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *res;
  int result;
  if((result = getaddrinfo(NULL, service, &hints, &res)) != 0)
  {
    fprintf(stderr, "libnet: Error: Failed to create server: %s\n", gai_strerror(result));
    free(server);
    return NULL;
  }

  for(struct addrinfo *p = res; p; p = p->ai_next)
  {
    if((server->socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
      continue;

    if(setsockopt(server->socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1)
    {
      fprintf(stderr, "libnet: Warn: Failed to set socket option: %s\n", strerror(errno));
      close(server->socket_fd);
      continue;
    }

    if(bind(server->socket_fd, p->ai_addr, p->ai_addrlen) == -1)
    {
      fprintf(stderr, "libnet: Warn: Failed to bind socket: %s\n", strerror(errno));
      close(server->socket_fd);
      continue;
    }

    if(listen(server->socket_fd, 4) == -1)
    {
      fprintf(stderr, "libnet: Warn: Failed to set socket to listening mode: %s\n", strerror(errno));
      close(server->socket_fd);
      continue;
    }

    fcntl(server->socket_fd, F_SETFL, fcntl(server->socket_fd, F_GETFL, 0) | O_NONBLOCK);
    freeaddrinfo(res);

    if((server->timer_fd = timerfd_create(CLOCK_MONOTONIC, 0)) == -1)
    {
      fprintf(stderr, "libnet: Error: Failed to create timer file descriptor: %s\n", strerror(errno));
      close(server->socket_fd);
      free(server);
    }

    fcntl(server->timer_fd, F_SETFL, fcntl(server->timer_fd, F_GETFL, 0) | O_NONBLOCK);

    struct itimerspec itimerspec;
    itimerspec.it_value.tv_sec = 0;
    itimerspec.it_value.tv_nsec = nsec;
    itimerspec.it_interval.tv_sec = 0;
    itimerspec.it_interval.tv_nsec = nsec;
    if(timerfd_settime(server->timer_fd, 0, &itimerspec, NULL) == -1)
    {
      fprintf(stderr, "libnet: Error: Failed to configure timer file descriptor: %s\n", strerror(errno));
      close(server->socket_fd);
      close(server->timer_fd);
      free(server);
    }

    if((server->epoll_fd = epoll_create1(0)) == -1)
    {
      fprintf(stderr, "libnet: Error: Failed to create epoll file descriptor: %s\n", strerror(errno));
      close(server->socket_fd);
      close(server->timer_fd);
      free(server);
      return NULL;
    }

    struct epoll_event epoll_event;

    epoll_event.events = EPOLLIN;
    epoll_event.data.ptr = &server->socket_fd;
    if(epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->socket_fd, &epoll_event) == -1)
    {
      fprintf(stderr, "libnet: Error: Failed to add socket to epoll file descriptor: %s\n", strerror(errno));
      close(server->epoll_fd);
      close(server->socket_fd);
      close(server->timer_fd);
      free(server);
      return NULL;
    }

    epoll_event.events = EPOLLIN;
    epoll_event.data.ptr = &server->timer_fd;
    if(epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->timer_fd, &epoll_event) == -1)
    {
      fprintf(stderr, "libnet: Error: Failed to add socket to epoll file descriptor: %s\n", strerror(errno));
      close(server->epoll_fd);
      close(server->socket_fd);
      close(server->timer_fd);
      free(server);
      return NULL;
    }

    LIST_INIT(&server->client_proxies);
    server->opaque = NULL;
    server->on_client_connected = NULL;
    server->on_client_disconnected = NULL;
    server->on_message_received = NULL;
    return server;
  }

  fprintf(stderr, "libnet: Error: Failed to create any socket\n");
  freeaddrinfo(res);
  free(server);
  return NULL;
}

void libnet_server_destroy(libnet_server_t server)
{
  while(!LIST_EMPTY(&server->client_proxies))
  {
    libnet_client_proxy_t client_proxy = LIST_FIRST(&server->client_proxies);
    socket_destroy(client_proxy->socket);
    free(client_proxy);
  }

  close(server->epoll_fd);
  close(server->socket_fd);
  close(server->timer_fd);
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

void libnet_server_run(libnet_server_t server)
{
  for(;;)
  {
    struct epoll_event epoll_events[32];
    int epoll_event_count;
    if((epoll_event_count = epoll_wait(server->epoll_fd, epoll_events, sizeof epoll_events / sizeof epoll_events[0], -1)) == -1)
    {
      fprintf(stderr, "libnet: Warn: Failed to wait on epoll file descriptor: %s\n", strerror(errno));
      return;
    }

    for(int i=0; i<epoll_event_count; ++i)
    {
      const struct epoll_event epoll_event = epoll_events[i];
      if(epoll_event.data.ptr == &server->socket_fd)
      {
        struct sockaddr_storage address;
        socklen_t address_len;
        int fd;
        while(address_len = sizeof address, (fd = accept(server->socket_fd, (struct sockaddr *)&address, &address_len)) != -1)
        {
          // Initialize client proxy.
          libnet_client_proxy_t client_proxy = malloc(sizeof *client_proxy);
          client_proxy->socket = socket_create_from_fd(fd);
          client_proxy->opaque = NULL;

          // Add to epoll fd.
          struct epoll_event epoll_event;
          epoll_event.events = EPOLLIN;
          epoll_event.data.ptr = client_proxy;
          if(epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, fd, &epoll_event) == -1)
          {
            fprintf(stderr, "libnet: Warn: Not accepting connection: Failed to add socket to epoll file descriptor: %s\n", strerror(errno));
            socket_destroy(client_proxy->socket);
            free(client_proxy);
            continue;
          }

          // Add to internal linked list.
          LIST_INSERT_HEAD(&server->client_proxies, client_proxy, entry);

          // Callback.
          if(server->on_client_connected)
            server->on_client_connected(server, client_proxy);
        }

        if(errno != EAGAIN && errno != EWOULDBLOCK)
          fprintf(stderr, "libnet: Warn: Failed to accept connection: %s\n", strerror(errno));
      }
      else if(epoll_event.data.ptr == &server->timer_fd)
      {
        uint64_t count;
        ssize_t n = read(server->timer_fd, &count, sizeof count);

        if(n == -1)
        {
          if(errno != EAGAIN && errno != EWOULDBLOCK)
            fprintf(stderr, "libnet: Warn: Error from timer: %s\n", strerror(errno));

          continue;
        }

        if(n != sizeof count)
        {
          fprintf(stderr, "libnet: Expected %zu bytes from timer. Got %zu bytes.\n", sizeof count, n);
          continue;
        }

        for(uint64_t i=0; i<count; ++i)
          server->on_update(server);
      }
      else
      {
        libnet_client_proxy_t client_proxy = epoll_event.data.ptr;

        int connection_closed = 0;

        if(epoll_event.events & EPOLLOUT)
          if(socket_update_try_send(&client_proxy->socket) != 0)
            fprintf(stderr, "libnet: Warn: Failed to send message: %s\n", strerror(errno));

        if(epoll_event.events & EPOLLIN)
          if(socket_update_try_recv(&client_proxy->socket, &connection_closed) != 0)
            fprintf(stderr, "libnet: Warn: Failed to send message: %s\n", strerror(errno));

        const struct libnet_message *message;

        size_t i = 0;
        while((message = socket_dequeue_message(&client_proxy->socket, &i)))
          if(server->on_message_received)
            server->on_message_received(server, client_proxy, message);

        socket_dequeue_message_end(&client_proxy->socket, i);

        // Stop polling for if we can send more data.
        if(client_proxy->socket.send_n == 0)
        {
          struct epoll_event epoll_event;
          epoll_event.events = EPOLLIN;
          epoll_event.data.ptr = client_proxy;
          if(epoll_ctl(server->epoll_fd, EPOLL_CTL_MOD, client_proxy->socket.fd, &epoll_event) == -1)
            fprintf(stderr, "libnet: Warn: Failed to change event mask on epoll file descriptor: %s\n", strerror(errno));
        }

        if(connection_closed)
        {
          // Callback.
          if(server->on_client_disconnected)
            server->on_client_disconnected(server, client_proxy);

          // Remove from internal linked list.
          LIST_REMOVE(client_proxy, entry);

          // Remove from epoll fd.
          struct epoll_event epoll_event = {0};
          if(epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, client_proxy->socket.fd, &epoll_event) == -1)
            fprintf(stderr, "libnet: Warn: Failed to remove socket from epoll file descriptor: %s\n", strerror(errno));

          // Destroy client proxy.
          socket_destroy(client_proxy->socket);
          free(client_proxy);
        }
      }
    }
  }
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
  // Start polling for if we can send more data.
  if(client_proxy->socket.send_n == 0)
  {
    struct epoll_event epoll_event;
    epoll_event.events = EPOLLIN | EPOLLOUT;
    epoll_event.data.ptr = client_proxy;
    if(epoll_ctl(server->epoll_fd, EPOLL_CTL_MOD, client_proxy->socket.fd, &epoll_event) == -1)
      fprintf(stderr, "libnet: Warn: Failed to change event mask on epoll file descriptor: %s\n", strerror(errno));
  }

  socket_enqueue_message(&client_proxy->socket, message);
}

