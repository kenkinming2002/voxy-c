#ifndef VOXY_PROTOCOL_CLIENT_H
#define VOXY_PROTOCOL_CLIENT_H

#include <libnet/message.h>

#include <libcommon/math/vector.h>

#include <stdint.h>
#include <stdlib.h>

/// Tag for message from client.
enum LIBNET_MESSAGE voxy_client_message_tag
{
  VOXY_CLIENT_MESSAGE_INPUT,
};

/// A message from client.
struct LIBNET_MESSAGE voxy_client_message
{
  struct libnet_message message;
  enum voxy_client_message_tag tag;
};

/// Sent from client to server for to update input state.
///
/// The 6 bits indicate if the corresponding direction keys is down. It is up to
/// client what physical keys they actually map to.
///
/// This message is only sent if there are changes in input state.
struct LIBNET_MESSAGE voxy_client_input_message
{
  struct voxy_client_message message;

  /// Keys.
  uint8_t key_left : 1;
  uint8_t key_right : 1;
  uint8_t key_back : 1;
  uint8_t key_front : 1;
  uint8_t key_bottom : 1;
  uint8_t key_top : 1;

  /// Mouse buttons.
  uint8_t mouse_button_left : 1;
  uint8_t mouse_button_right : 1;

  /// Mouse motion.
  fvec2_t mouse_motion;
};

/// Try to cast struct libnet_message to struct voxy_chunk_update_message.
///
/// Return NULL on failure.
static inline struct voxy_client_input_message *voxy_get_client_input_message(const struct libnet_message *message)
{
  if(message->size != LIBNET_MESSAGE_SIZE(struct voxy_client_input_message))
    return NULL;

  struct voxy_client_input_message *_message = (struct voxy_client_input_message *)message;
  if(_message->message.tag != VOXY_CLIENT_MESSAGE_INPUT)
    return NULL;

  return _message;
}


#endif // VOXY_PROTOCOL_CLIENT_H
