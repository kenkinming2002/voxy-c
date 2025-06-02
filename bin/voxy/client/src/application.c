#include "application.h"
#include "camera/main.h"
#include "chunk/block/manager.h"
#include "input/input.h"
#include "mod/mod.h"
#include "render/world.h"

#include <ui/manager.h>

#include <voxy/protocol/client.h>

#include <libgfx/render.h>

#include <libgfx/window.h>
#include <libgfx/window.h>

#include <libui/ui.h>

#include <libcore/profile.h>

#include <alloca.h>
#include <stdio.h>
#include <string.h>

int application_init(struct application *application, int argc, char *argv[])
{
  if(argc < 4)
  {
    fprintf(stderr, "Usage: %s SERVICE CERT KEY PLAYER_NAME [MOD]...", argv[0]);
    return -1;
  }

  window_init("client", 1024, 720);
  libnet_client_init(argv[1], argv[2]);

  struct voxy_client_login_message *message = alloca(sizeof *message + strlen(argv[3]));
  message->message.tag = VOXY_CLIENT_MESSAGE_LOGIN;
  message->message.message.size = LIBNET_MESSAGE_SIZE(*message) + strlen(argv[3]);
  memcpy(message->player_name, argv[3], strlen(argv[3]));
  libnet_client_send_message(&message->message.message);

  libnet_client_set_opaque(application);
  libnet_client_set_on_message_received(application_on_message_received);

  main_camera_init();

  struct voxy_context context = application_get_context(application);
  for(int i=4; i<argc; ++i)
    mod_load(argv[i], &context);

  world_renderer_init();

  return 0;
}

struct voxy_context application_get_context(struct application *application)
{
  struct voxy_context context;
  return context;
}

void application_fini(struct application *application)
{
  libnet_client_deinit();
}

static void application_update_network(struct application *application)
{
  profile_scope;
  libnet_client_update();
}

static void application_update(struct application *application)
{
  profile_scope;

  window_update();

  application_update_network(application);

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

void application_run(struct application *application)
{
  while(!window_should_close())
    application_update(application);
}

void application_on_message_received(const struct libnet_message *message)
{
  struct application *application = libnet_client_get_opaque();
  main_camera_on_message_received(message);
  block_manager_on_message_received(message);
  entity_manager_on_message_received(message);
}

