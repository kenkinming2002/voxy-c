#ifndef CAMERA_MAIN_H
#define CAMERA_MAIN_H

#include <chunk/entity/manager.h>
#include <libgfx/camera.h>

#include <libnet/client.h>

void main_camera_init(void);
void main_camera_update(void);
void main_camera_on_message_received(const struct libnet_message *_message);

struct camera get_main_camera(void);
entity_handle_t get_main_camera_target(void);

#endif // CAMERA_MAIN_H
