#ifndef VOXY_PROTOCOL_SERVER_H
#define VOXY_PROTOCOL_SERVER_H

#include <voxy/config.h>

#include <libnet/message.h>
#include <libmath/vector.h>
#include <stdint.h>

/// Tag for message from server.
enum LIBNET_MESSAGE voxy_server_message_tag
{
  VOXY_SERVER_MESSAGE_CHUNK_UPDATE,
  VOXY_SERVER_MESSAGE_CHUNK_REMOVE,

  VOXY_SERVER_MESSAGE_ENTITY_UPDATE,
  VOXY_SERVER_MESSAGE_ENTITY_REMOVE,

  VOXY_SERVER_MESSAGE_CAMERA_FOLLOW_ENTITY,
};

/// A message from server.
struct LIBNET_MESSAGE voxy_server_message
{
  struct libnet_message message;
  enum voxy_server_message_tag tag;
};

/// Sent from server to client to update an entire chunk.
struct LIBNET_MESSAGE voxy_server_chunk_update_message
{
  struct voxy_server_message message;

  ivec3_t position;
  uint8_t block_ids[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH];
  uint8_t block_light_levels[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH / 2];
};

/// Sent from server to client to remove an entire chunk.
struct LIBNET_MESSAGE voxy_server_chunk_remove_message
{
  struct voxy_server_message message;

  ivec3_t position;
};

/// Sent from server to client to update an entity.
struct LIBNET_MESSAGE voxy_server_entity_update_message
{
  struct voxy_server_message message;

  uint32_t handle;

  uint8_t id;
  fvec3_t position;
  fvec3_t rotation;
};

/// Sent from server to client to remove an entity.
struct LIBNET_MESSAGE voxy_server_entity_remove_message
{
  struct voxy_server_message message;

  uint32_t handle;
};

/// Sent from server to client to set entity that camera should follow
struct LIBNET_MESSAGE voxy_server_camera_follow_entity_message
{
  struct voxy_server_message message;

  uint32_t handle;
};

/// Try to cast struct libnet_message to struct voxy_chunk_update_message.
///
/// Return NULL on failure.
static inline struct voxy_server_chunk_update_message *voxy_get_server_chunk_update_message(const struct libnet_message *message)
{
  if(message->size != LIBNET_MESSAGE_SIZE(struct voxy_server_chunk_update_message))
    return NULL;

  struct voxy_server_chunk_update_message *_message = (struct voxy_server_chunk_update_message *)message;
  if(_message->message.tag != VOXY_SERVER_MESSAGE_CHUNK_UPDATE)
    return NULL;

  return _message;
}

/// Try to cast struct libnet_message to struct voxy_chunk_remove_message.
///
/// Return NULL on failure.
static inline struct voxy_server_chunk_remove_message *voxy_get_server_chunk_remove_message(const struct libnet_message *message)
{
  if(message->size != LIBNET_MESSAGE_SIZE(struct voxy_server_chunk_remove_message))
    return NULL;

  struct voxy_server_chunk_remove_message *_message = (struct voxy_server_chunk_remove_message *)message;
  if(_message->message.tag != VOXY_SERVER_MESSAGE_CHUNK_REMOVE)
    return NULL;

  return _message;
}

/// Try to cast struct libnet_message to struct voxy_entity_update_message.
///
/// Return NULL on failure.
static inline struct voxy_server_entity_update_message *voxy_get_server_entity_update_message(const struct libnet_message *message)
{
  if(message->size != LIBNET_MESSAGE_SIZE(struct voxy_server_entity_update_message))
    return NULL;

  struct voxy_server_entity_update_message *_message = (struct voxy_server_entity_update_message *)message;
  if(_message->message.tag != VOXY_SERVER_MESSAGE_ENTITY_UPDATE)
    return NULL;

  return _message;
}

/// Try to cast struct libnet_message to struct voxy_entity_remove_message.
///
/// Return NULL on failure.
static inline struct voxy_server_entity_remove_message *voxy_get_server_entity_remove_message(const struct libnet_message *message)
{
  if(message->size != LIBNET_MESSAGE_SIZE(struct voxy_server_entity_remove_message))
    return NULL;

  struct voxy_server_entity_remove_message *_message = (struct voxy_server_entity_remove_message *)message;
  if(_message->message.tag != VOXY_SERVER_MESSAGE_ENTITY_REMOVE)
    return NULL;

  return _message;
}

/// Try to cast struct libnet_message to struct voxy_camera_follow_entity_message.
///
/// Return NULL on failure.
static inline struct voxy_server_camera_follow_entity_message *voxy_get_server_camera_follow_entity_message(const struct libnet_message *message)
{
  if(message->size != LIBNET_MESSAGE_SIZE(struct voxy_server_camera_follow_entity_message))
    return NULL;

  struct voxy_server_camera_follow_entity_message *_message = (struct voxy_server_camera_follow_entity_message *)message;
  if(_message->message.tag != VOXY_SERVER_MESSAGE_CAMERA_FOLLOW_ENTITY)
    return NULL;

  return _message;
}

#endif // VOXY_PROTOCOL_SERVER_H
