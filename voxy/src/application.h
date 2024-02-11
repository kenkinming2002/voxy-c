#ifndef VOXY_APPLICATION_H
#define VOXY_APPLICATION_H

#include <core/window.h>
#include "renderer_world.h"

#include "application_main_game.h"

struct application
{
  struct renderer_world renderer_world;

  struct application_main_game main_game;
};

int application_init(struct application *application);
void application_fini(struct application *application);

void application_update(struct application *application, float dt);
void application_render(struct application *application);
void application_run(struct application *application);

#endif // VOXY_APPLICATION_H
