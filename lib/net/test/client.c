#include <libnet/client.h>

#include "protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

static void on_message_received(libnet_client_t client, const struct libnet_message *message)
{
  (void)client;

  const struct test_message *my_message = (const struct test_message *)message;
  printf("Message received: %.*s\n", message->size, my_message->msg);
}

int main(int argc, char *argv[])
{
  if(argc != 4)
  {
    fprintf(stderr, "Usage : %s NODE SERVICE MESSAGE\n", argv[0]);
    return EXIT_FAILURE;
  }

  libnet_client_t client = libnet_client_create(argv[1], argv[2]);
  if(!client)
    return EXIT_FAILURE;

  libnet_client_set_on_message_received(client, on_message_received);

  const char *s = argv[3];
  size_t n = strlen(s);

  struct test_message *message = malloc(sizeof *message + n);
  message->message.size = n;
  memcpy(message->msg, s, n);

  for(;;)
  {
    libnet_client_send_message(client, &message->message);
    if(libnet_client_update(client))
      break;

    sleep(1);
  }

  libnet_client_destroy(client);
}
