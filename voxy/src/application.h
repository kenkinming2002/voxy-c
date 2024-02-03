#ifndef VOXY_APPLICATION_H
#define VOXY_APPLICATION_H

#include "application_main_game.h"
#include "renderer.h"
#include "window.h"

struct application
{
  struct window   window;
  struct application_main_game main_game;
};

int application_init(struct application *application);
void application_fini(struct application *application);

void application_update(struct application *application, struct input *input, float dt);
void application_render(struct application *application, int width, int height);
void application_run(struct application *application);

#endif // VOXY_APPLICATION_H
