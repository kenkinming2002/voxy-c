#include "application.h"

#include <libcommon/core/window.h>
#include <libcommon/core/delta_time.h>

#include <stdio.h>

int application_init(struct application *application, int argc, char *argv[])
{
  if(argc != 3)
  {
    fprintf(stderr, "Usage: %s NODE SERVICE", argv[0]);
    return -1;
  }

  window_init("client", 1024, 720);

  block_registry_init(&application->block_registry);
  entity_registry_init(&application->entity_registry);

  block_registry_register_block(&application->block_registry, (struct block_info){
    .mod = "base",
    .name = "air",
    .type = BLOCK_TYPE_INVISIBLE,
    .textures = {0},
  });

  block_registry_register_block(&application->block_registry, (struct block_info){
    .mod = "base",
    .name = "ether",
    .type = BLOCK_TYPE_INVISIBLE,
    .textures = {0},
  });

  block_registry_register_block(&application->block_registry, (struct block_info){
    .mod = "base",
    .name = "stone",
    .type = BLOCK_TYPE_OPAQUE,
    .textures[DIRECTION_LEFT]   = "bin/mod/assets/textures/stone.png",
    .textures[DIRECTION_RIGHT]  = "bin/mod/assets/textures/stone.png",
    .textures[DIRECTION_BACK]   = "bin/mod/assets/textures/stone.png",
    .textures[DIRECTION_FRONT]  = "bin/mod/assets/textures/stone.png",
    .textures[DIRECTION_BOTTOM] = "bin/mod/assets/textures/stone.png",
    .textures[DIRECTION_TOP]    = "bin/mod/assets/textures/stone.png",
  });

  block_registry_register_block(&application->block_registry, (struct block_info){
    .mod = "base",
    .name = "grass",
    .type = BLOCK_TYPE_OPAQUE,
    .textures[DIRECTION_LEFT]   = "bin/mod/assets/textures/grass_side.png",
    .textures[DIRECTION_RIGHT]  = "bin/mod/assets/textures/grass_side.png",
    .textures[DIRECTION_BACK]   = "bin/mod/assets/textures/grass_side.png",
    .textures[DIRECTION_FRONT]  = "bin/mod/assets/textures/grass_side.png",
    .textures[DIRECTION_BOTTOM] = "bin/mod/assets/textures/grass_bottom.png",
    .textures[DIRECTION_TOP]    = "bin/mod/assets/textures/grass_top.png",
  });

  if(!(application->client = libnet_client_create(argv[1], argv[2])))
    goto error0;

  if(input_manager_init(&application->input_manager) != 0)
    goto error1;

  if(camera_manager_init(&application->camera_manager) != 0)
    goto error2;

  if(chunk_manager_init(&application->chunk_manager) != 0)
    goto error3;

  entity_manager_init(&application->entity_manager);

  if(world_renderer_init(&application->world_renderer, &application->block_registry) != 0)
    goto error4;

  libnet_client_set_opaque(application->client, application);
  libnet_client_set_on_message_received(application->client, application_on_message_received);
  return 0;

error4:
  entity_manager_fini(&application->entity_manager);
  chunk_manager_fini(&application->chunk_manager);
error3:
  camera_manager_fini(&application->camera_manager);
error2:
  input_manager_fini(&application->input_manager);
error1:
  libnet_client_destroy(application->client);
error0:
  block_registry_fini(&application->block_registry);
  return -1;
}

void application_fini(struct application *application)
{
  world_renderer_fini(&application->world_renderer);
  entity_manager_fini(&application->entity_manager);
  chunk_manager_fini(&application->chunk_manager);
  camera_manager_fini(&application->camera_manager);
  input_manager_fini(&application->input_manager);
  libnet_client_destroy(application->client);
  entity_registry_fini(&application->entity_registry);
  block_registry_fini(&application->block_registry);
}

void application_run(struct application *application)
{
  while(!window_should_close())
  {
    window_update();

    libnet_client_update(application->client);
    input_manager_update(&application->input_manager, application->client);
    camera_manager_update(&application->camera_manager);
    chunk_manager_update(&application->chunk_manager);
    world_renderer_update(&application->world_renderer, &application->block_registry, &application->chunk_manager);

    glViewport(0, 0, window_size.x, window_size.y);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    world_renderer_render(&application->world_renderer, &application->camera_manager.camera);

    window_present();
  }
}

void application_on_message_received(libnet_client_t client, const struct libnet_message *message)
{
  struct application *application = libnet_client_get_opaque(client);
  camera_manager_on_message_received(&application->camera_manager, client, message);
  chunk_manager_on_message_received(&application->chunk_manager, client, message);
}

