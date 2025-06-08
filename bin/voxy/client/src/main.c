#include "camera/main.h"
#include "chunk/block/manager.h"
#include "input/input.h"
#include "mod/mod.h"
#include "render/world.h"
#include "ui/manager.h"

#include <voxy/protocol/client.h>

#include <libcore/profile.h>
#include <libgfx/render.h>
#include <libgfx/window.h>
#include <libnet/client.h>
#include <libui/ui.h>

#include <stdio.h>
#include <string.h>

static void on_message_received(const struct libnet_message *message)
{
  main_camera_on_message_received(message);
  block_manager_on_message_received(message);
  entity_manager_on_message_received(message);
}

static void init(int argc, char *argv[])
{
  window_init("client", 1024, 720);

  libnet_client_run(argv[1], argv[2]);

  struct voxy_client_login_message *message = calloc(1, sizeof *message + strlen(argv[3]));
  message->message.tag = VOXY_CLIENT_MESSAGE_LOGIN;
  message->message.message.size = LIBNET_MESSAGE_SIZE(*message) + strlen(argv[3]);
  memcpy(message->player_name, argv[3], strlen(argv[3]));
  libnet_client_send_message(&message->message.message);

  main_camera_init();

  for(int i=4; i<argc; ++i)
    mod_load(argv[i]);

  world_renderer_init();

}

static void update(void)
{
  profile_scope;

  window_update();

  input_update();
  main_camera_update();
  world_renderer_update();
  ui_manager_update();

  glViewport(0, 0, window_size.x, window_size.y);
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  render_world();
  render_end();
  ui_render();

  window_present();
}

static void run(void)
{
  for(;;)
  {
    if(window_should_close())
      return;

    update();

    struct libnet_client_event event;
    while(libnet_client_poll_event(&event))
      switch(event.type)
      {
      case LIBNET_CLIENT_EVENT_SERVER_DISCONNECTED:
        return;
      case LIBNET_CLIENT_EVENT_MESSAGE_RECEIVED:
        on_message_received(event.message);
        break;
      }
  }
}

int main(int argc, char *argv[])
{
  if(argc < 4)
  {
    fprintf(stderr, "Usage: %s SERVICE CERT KEY PLAYER_NAME [MOD]...", argv[0]);
    return -1;
  }

  init(argc, argv);
  run();
}
