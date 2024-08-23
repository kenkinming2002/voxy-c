#ifndef LIBNET_MESSAGE_H
#define LIBNET_MESSAGE_H

#include <stdint.h>

/// We have to specify alignment 1 for any message structure. This is because we
/// use create pointer directly into the network receive buffer which may be
/// unaligned.
#define LIBNET_MESSAGE __attribute__((packed,aligned(1)))

struct LIBNET_MESSAGE libnet_message
{
  uint32_t size;
};

#endif // LIBNET_MESSAGE_H
