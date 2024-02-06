#ifndef VOXY_APPLICATION_MAIN_GAME_H
#define VOXY_APPLICATION_MAIN_GAME_H

#include "resource_pack.h"
#include "world.h"
#include "world_generator.h"
#include "renderer_world.h"
#include "renderer_ui.h"
#include "window.h"

struct application_main_game
{
  struct resource_pack resource_pack;

  struct world           world;
  struct world_generator world_generator;
};

int application_main_game_init(struct application_main_game *application_main_game);
void application_main_game_fini(struct application_main_game *application_main_game);

void application_main_game_update(struct application_main_game *application_main_game, struct window *window, float dt);
void application_main_game_update_ui(struct application_main_game *application_main_game, struct window *window);

void application_main_game_render(struct application_main_game *application_main_game, struct window *window, struct renderer_world *renderer_world, struct renderer_ui *renderer_ui);
void application_main_game_render_ui(struct application_main_game *application_main_game, struct window *window, struct renderer_ui *renderer_ui);

#endif // VOXY_APPLICATION_MAIN_GAME_H

