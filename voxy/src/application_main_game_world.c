#include "application_main_game.h"

#include "world_update_chunk_generate.h"
#include "world_update_player_control.h"
#include "world_update_light.h"
#include "world_update_chunk_mesh.h"

#include "renderer_world.h"

void application_main_game_update_world(struct application_main_game *application_main_game, struct window *window, float dt)
{
  world_update_chunk_generate(&application_main_game->world, &application_main_game->resource_pack, &application_main_game->world_generator);
  world_update_player_control(&application_main_game->world, &application_main_game->resource_pack, window, dt);
  world_update_light(&application_main_game->world, &application_main_game->resource_pack);
  world_update_chunk_mesh(&application_main_game->world, &application_main_game->resource_pack);
}

void application_main_game_render_world(struct application_main_game *application_main_game, struct window *window, struct renderer_world *renderer_world)
{
  renderer_world_render(renderer_world, window, &application_main_game->world, &application_main_game->resource_pack);
}
