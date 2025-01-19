#include "application.h"

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

  voxy_block_registry_init(&application->block_registry);
  voxy_entity_registry_init(&application->entity_registry);
  voxy_item_registry_init(&application->item_registry);

  if(!(application->client = libnet_client_create(argv[1], argv[2])))
    goto error0;

  struct voxy_client_login_message *message = alloca(sizeof *message + strlen(argv[3]));
  message->message.tag = VOXY_CLIENT_MESSAGE_LOGIN;
  message->message.message.size = LIBNET_MESSAGE_SIZE(*message) + strlen(argv[3]);
  memcpy(message->player_name, argv[3], strlen(argv[3]));
  libnet_client_send_message(application->client, &message->message.message);

  libnet_client_set_opaque(application->client, application);
  libnet_client_set_on_message_received(application->client, application_on_message_received);

  if(input_manager_init(&application->input_manager) != 0)
    goto error1;

  if(camera_manager_init(&application->camera_manager) != 0)
    goto error2;

  if(chunk_manager_init(&application->chunk_manager) != 0)
    goto error3;

  entity_manager_init(&application->entity_manager);

  mod_manager_init(&application->mod_manager);

  struct voxy_context context = application_get_context(application);
  for(int i=4; i<argc; ++i)
    if(mod_manager_load(&application->mod_manager, argv[i], &context) != 0)
      goto error5;

  if(world_renderer_init(&application->world_renderer, &application->block_registry) != 0)
    goto error5;

  return 0;

error5:
  mod_manager_fini(&application->mod_manager, &context);
  entity_manager_fini(&application->entity_manager);
  chunk_manager_fini(&application->chunk_manager);
error3:
  camera_manager_fini(&application->camera_manager);
error2:
  input_manager_fini(&application->input_manager);
error1:
  libnet_client_destroy(application->client);
error0:
  voxy_item_registry_fini(&application->item_registry);
  voxy_entity_registry_fini(&application->entity_registry);
  voxy_block_registry_fini(&application->block_registry);
  return -1;
}

struct voxy_context application_get_context(struct application *application)
{
  struct voxy_context context;
  context.block_registry = &application->block_registry;
  context.entity_registry = &application->entity_registry;
  context.item_registry = &application->item_registry;
  return context;
}

void application_fini(struct application *application)
{
  struct voxy_context context = application_get_context(application);

  world_renderer_fini(&application->world_renderer);
  mod_manager_fini(&application->mod_manager, &context);
  entity_manager_fini(&application->entity_manager);
  chunk_manager_fini(&application->chunk_manager);
  camera_manager_fini(&application->camera_manager);
  input_manager_fini(&application->input_manager);
  libnet_client_destroy(application->client);
  voxy_item_registry_fini(&application->item_registry);
  voxy_entity_registry_fini(&application->entity_registry);
  voxy_block_registry_fini(&application->block_registry);
}

static void application_update_network(struct application *application)
{
  profile_begin();
  libnet_client_update(application->client);
  profile_end();
}

static void application_update(struct application *application)
{
  profile_begin();

  window_update();

  application_update_network(application);

  input_manager_update(&application->input_manager, application->client);
  camera_manager_update(&application->camera_manager, &application->entity_manager);

  chunk_manager_update(&application->chunk_manager);
  entity_manager_update(&application->entity_manager);

  world_renderer_update(&application->world_renderer, &application->block_registry, &application->chunk_manager, &application->camera_manager);

  ui_manager_update();

  glViewport(0, 0, window_size.x, window_size.y);
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  world_renderer_render(&application->world_renderer, &application->entity_registry, &application->entity_manager, &application->camera_manager);
  render_end();

  ui_render();

  window_present();

  profile_end();
}

void application_run(struct application *application)
{
  while(!window_should_close())
    application_update(application);
}

void application_on_message_received(libnet_client_t client, const struct libnet_message *message)
{
  struct application *application = libnet_client_get_opaque(client);
  camera_manager_on_message_received(&application->camera_manager, client, message);
  chunk_manager_on_message_received(&application->chunk_manager, client, message);
  entity_manager_on_message_received(&application->entity_manager, client, message);
}

