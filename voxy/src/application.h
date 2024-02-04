#ifndef VOXY_APPLICATION_H
#define VOXY_APPLICATION_H

#include "window.h"
#include "renderer_world.h"
#include "renderer_ui.h"

#include "application_main_game.h"

struct application
{
  struct window window;
  struct renderer_world renderer_world;
  struct renderer_ui    renderer_ui;

  struct application_main_game main_game;
};

int application_init(struct application *application);
void application_fini(struct application *application);

void application_update(struct application *application, int width, int height, bool *cursor, struct input *input, float dt);
void application_render(struct application *application, int width, int height);
void application_run(struct application *application);

#endif // VOXY_APPLICATION_H
