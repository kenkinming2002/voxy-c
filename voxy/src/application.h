#ifndef VOXY_APPLICATION_H
#define VOXY_APPLICATION_H

#include "renderer.h"
#include "resource_pack.h"
#include "window.h"
#include "world.h"
#include "world_generator.h"

struct application
{
  struct resource_pack resource_pack;

  struct window   window;
  struct renderer renderer;

  struct world           world;
  struct world_generator world_generator;
};

int application_init(struct application *application);
void application_fini(struct application *application);

void application_update(struct application *application, float dt);
void application_render(struct application *application);
void application_run(struct application *application);

#endif // VOXY_APPLICATION_H
