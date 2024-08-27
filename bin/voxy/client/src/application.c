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

  if(!(application->client = libnet_client_create(argv[1], argv[2])))
    goto error0;

  if(chunk_manager_init(&application->chunk_manager) != 0)
    goto error1;

  application->camera.fovy = M_PI / 2.0f;
  application->camera.near = 0.1f;
  application->camera.far = 1000.0f;
  application->camera.aspect = (float)window_size.x / (float)window_size.y;
  application->camera.transform.translation = fvec3_zero();
  application->camera.transform.rotation = fvec3_zero();

  if(world_renderer_init(&application->world_renderer) != 0)
    goto error2;

  libnet_client_set_opaque(application->client, application);
  libnet_client_set_on_message_received(application->client, application_on_message_received);
  return 0;

error2:
  chunk_manager_fini(&application->chunk_manager);
error1:
  libnet_client_destroy(application->client);
error0:
  return -1;
}

void application_fini(struct application *application)
{
  world_renderer_fini(&application->world_renderer);
  chunk_manager_fini(&application->chunk_manager);
  libnet_client_destroy(application->client);
}

void application_run(struct application *application)
{
  while(!window_should_close())
  {
    window_update();

    const float dt = get_delta_time();

    libnet_client_update(application->client);
    chunk_manager_update(&application->chunk_manager);
    world_renderer_update(&application->world_renderer, &application->chunk_manager);

    // Camera control system. FIXME: Refactor me.
    {
      application->camera.aspect = (float)window_size.x / (float)window_size.y;
      application->camera.transform.rotation.yaw   +=  mouse_motion.x * 0.002f;
      application->camera.transform.rotation.pitch += -mouse_motion.y * 0.002f;

      fvec3_t axis = fvec3_zero();

      if(input_state(KEY_A)) { axis.x -= 1.0f; }
      if(input_state(KEY_D)) { axis.x += 1.0f; }
      if(input_state(KEY_S)) { axis.y -= 1.0f; }
      if(input_state(KEY_W)) { axis.y += 1.0f; }
      if(input_state(KEY_SHIFT)) { axis.z -= 1.0f; }
      if(input_state(KEY_SPACE)) { axis.z += 1.0f; }

      application->camera.transform = transform_local_translate(application->camera.transform, fvec3_mul_scalar(axis, dt * 10.0f));
    }

    glViewport(0, 0, window_size.x, window_size.y);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    world_renderer_render(&application->world_renderer, &application->camera);

    window_present();
  }
}

void application_on_message_received(libnet_client_t client, const struct libnet_message *message)
{
  struct application *application = libnet_client_get_opaque(client);
  chunk_manager_on_message_received(&application->chunk_manager, client, message);
}

