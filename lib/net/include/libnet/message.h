#ifndef LIBNET_MESSAGE_H
#define LIBNET_MESSAGE_H

#include <stdint.h>

/// We have to specify alignment 1 for any message structure. This is because we
/// use create pointer directly into the network receive buffer which may be
/// unaligned.
#define LIBNET_MESSAGE __attribute__((packed,aligned(1)))

/// Header of any libnet messaage.
struct LIBNET_MESSAGE libnet_message
{
  uint32_t size;
};

/// Compute sizeof of libnet message.
///
/// This correctly ignores the size of libnet message header.
#define LIBNET_MESSAGE_SIZE(v) (sizeof(v) - sizeof(struct libnet_message))

#endif // LIBNET_MESSAGE_H
