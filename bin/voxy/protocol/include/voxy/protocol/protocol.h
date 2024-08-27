#ifndef VOXY_PROTOCOL_PROTOCOL_H
#define VOXY_PROTOCOL_PROTOCOL_H

#include <libnet/message.h>
#include <libcommon/math/vector.h>
#include <stdint.h>

#include "chunk.h"

/// Tag for message from server.
enum LIBNET_MESSAGE voxy_server_message_tag
{
  VOXY_MESSAGE_CHUNK_UPDATE,
};

/// A message from server.
struct LIBNET_MESSAGE voxy_server_message
{
  struct libnet_message message;
  enum voxy_server_message_tag tag;
};

/// Sent from server to client to update an entire chunk.
struct LIBNET_MESSAGE voxy_chunk_update_message
{
  struct voxy_server_message message;

  ivec3_t position;
  uint8_t block_ids[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH];
  uint8_t block_light_levels[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH / 2];
};

/// Try to cast struct libnet_message to struct voxy_chunk_update_message.
///
/// Return NULL on failure.
static inline struct voxy_chunk_update_message *voxy_get_chunk_update_message(const struct libnet_message *message)
{
  if(message->size != LIBNET_MESSAGE_SIZE(struct voxy_chunk_update_message))
    return NULL;

  struct voxy_chunk_update_message *_message = (struct voxy_chunk_update_message *)message;
  if(_message->message.tag != VOXY_MESSAGE_CHUNK_UPDATE)
    return NULL;

  return _message;
}

#endif // VOXY_PROTOCOL_PROTOCOL_H
