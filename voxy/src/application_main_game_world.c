#include "application_main_game.h"

#include "renderer_world.h"

void application_main_game_update_world(struct application_main_game *application_main_game, struct window *window, float dt)
{
  world_update(&application_main_game->world, &application_main_game->world_generator, &application_main_game->resource_pack, window, dt);
}

void application_main_game_render_world(struct application_main_game *application_main_game, struct window *window, struct renderer_world *renderer_world)
{
  renderer_world_render(renderer_world, window, &application_main_game->world, &application_main_game->resource_pack);
}
