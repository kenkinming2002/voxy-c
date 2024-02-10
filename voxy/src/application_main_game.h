#ifndef VOXY_APPLICATION_MAIN_GAME_H
#define VOXY_APPLICATION_MAIN_GAME_H

#include <types/mod.h>
#include <types/mod_assets.h>

#include "world_generator.h"

struct application_main_game
{
  struct mod        mod;
  struct mod_assets mod_assets;

  struct world           world;
  struct world_generator world_generator;
};

struct window;
struct renderer_world;
struct renderer_ui;

int application_main_game_init(struct application_main_game *application_main_game);
void application_main_game_fini(struct application_main_game *application_main_game);

void application_main_game_update(struct application_main_game *application_main_game, struct window *window, float dt);
void application_main_game_update_world(struct application_main_game *application_main_game, struct window *window, float dt);
void application_main_game_update_ui   (struct application_main_game *application_main_game, struct window *window);

void application_main_game_render      (struct application_main_game *application_main_game, struct window *window, struct renderer_world *renderer_world, struct renderer_ui *renderer_ui);
void application_main_game_render_world(struct application_main_game *application_main_game, struct window *window, struct renderer_world *renderer_world);
void application_main_game_render_ui   (struct application_main_game *application_main_game, struct window *window, struct renderer_ui *renderer_ui);

#endif // VOXY_APPLICATION_MAIN_GAME_H

