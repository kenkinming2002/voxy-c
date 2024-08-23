#include <libnet/server.h>

#include "protocol.h"

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

struct my_message
{
  struct libnet_message message;
  char data[];
};

static void on_client_connected(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  printf("Client connected %p => %p\n", server, client_proxy);
}

static void on_client_disconnected(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  printf("Client disconnected %p => %p\n", server, client_proxy);
}

static void on_message_received(libnet_server_t server, libnet_client_proxy_t client_proxy, const struct libnet_message *message)
{
  (void)client_proxy;

  const struct test_message *my_message = (const struct test_message *)message;
  printf("Message received: %.*s\n", message->size, my_message->msg);
  libnet_server_send_message_all(server, message);
}

int main(int argc, char *argv[])
{
  if(argc != 2)
  {
    fprintf(stderr, "Usage : %s SERVICE\n", argv[0]);
    return EXIT_FAILURE;
  }

  libnet_server_t server = libnet_server_create(argv[1]);
  if(!server)
    return EXIT_FAILURE;

  libnet_server_set_on_client_connected(server, on_client_connected);
  libnet_server_set_on_client_disconnected(server, on_client_disconnected);
  libnet_server_set_on_message_received(server, on_message_received);

  for(;;)
  {
    libnet_server_update(server);
    sleep(1);
  }

  libnet_server_destroy(server);
}
