#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <libnet/message.h>

struct LIBNET_MESSAGE test_message
{
  struct libnet_message message;
  char msg[];
};

#endif // PROTOCOL_H
