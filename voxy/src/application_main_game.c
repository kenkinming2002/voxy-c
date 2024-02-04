#include "application_main_game.h"

#include <stdio.h>
#include <stdlib.h>

#define RESOURCE_PACK_FILEPATH "resource_pack/resource_pack.so"

#define CHECK(expr) if((expr) != 0) { fprintf(stderr, "%s:%d: ERROR: %s != 0\n", __FILE__, __LINE__, #expr); exit(EXIT_FAILURE); }

int application_main_game_init(struct application_main_game *application_main_game)
{
  seed_t seed = time(NULL);

  CHECK(resource_pack_load(&application_main_game->resource_pack, RESOURCE_PACK_FILEPATH));

  world_init(&application_main_game->world, seed);
  world_generator_init(&application_main_game->world_generator, seed);
  return 0;
}

void application_main_game_fini(struct application_main_game *application_main_game)
{
  world_generator_fini(&application_main_game->world_generator);
  world_fini(&application_main_game->world);
  resource_pack_unload(&application_main_game->resource_pack);
}

void application_main_game_update(struct application_main_game *application_main_game, struct input *input, float dt)
{
  world_update(&application_main_game->world, &application_main_game->world_generator, &application_main_game->resource_pack, input, dt);
}

static inline float minf(float a, float b)
{
  return a < b ? a : b;
}

#define UI_HOTBAR_COUNT 9
#define UI_HOTBAR_COLOR_BACKGROUND fvec4(0.9f, 0.9f, 0.9f, 0.3f)
#define UI_HOTBAR_COLOR_SELECTED   fvec4(0.95f, 0.75f, 0.75f, 0.8f)
#define UI_HOTBAR_COLOR_DEFAULT    fvec4(0.95f, 0.95f, 0.95f, 0.7f)
#define UI_TEXT_SIZE 28

static inline void application_main_game_render_ui(struct application_main_game *application_main_game, int width, int height, struct renderer_ui *renderer_ui)
{
  if(!application_main_game->world.player.spawned)
    return;

  struct resource_pack *resource_pack = &application_main_game->resource_pack;
  struct world         *world         = &application_main_game->world;
  struct player        *player        = &world->player;

  renderer_ui_begin(renderer_ui, fvec2(width, height));
  {
    //////////////////
    /// 0: Metrics ///
    //////////////////
    const float base       = minf(width, height);
    const float margin     = base * 0.008f;
    const float cell_width = base * 0.05f;
    const float cell_sep   = base * 0.006f;
    const float round      = base * 0.006f;

    /////////////////
    /// 1: Hotbar ///
    /////////////////
    const float   hotbar_width     = cell_sep + UI_HOTBAR_COUNT * (cell_sep + cell_width);
    const float   hotbar_height    = 2 * cell_sep + cell_width;
    const fvec2_t hotbar_position  = fvec2((width - hotbar_width) * 0.5f, margin);
    const fvec2_t hotbar_dimension = fvec2(hotbar_width, hotbar_height);

    renderer_ui_draw_quad_rounded(renderer_ui, hotbar_position, hotbar_dimension, round, UI_HOTBAR_COLOR_BACKGROUND);

    for(int i=0; i<UI_HOTBAR_COUNT; ++i)
    {
      const fvec2_t cell_position  = fvec2_add(hotbar_position, fvec2(cell_sep + i * (cell_sep + cell_width), cell_sep));
      const fvec2_t cell_dimension = fvec2(cell_width, cell_width);
      const fvec4_t cell_color     = player->inventory.hotbar_selection == i ? UI_HOTBAR_COLOR_SELECTED : UI_HOTBAR_COLOR_DEFAULT;

      renderer_ui_draw_quad_rounded(renderer_ui, cell_position, cell_dimension, round, cell_color);
    }

    //////////////////
    /// 2: Tooltip ///
    //////////////////
    const fvec2_t tooltip_position = fvec2(width * 0.5f, hotbar_position.y + hotbar_dimension.y + margin);

    uint8_t     tooltip_item_id   = player->inventory.hotbar_items[player->inventory.hotbar_selection];
    const char *tooltip_item_name = tooltip_item_id != ITEM_NONE ? application_main_game->resource_pack.item_infos[tooltip_item_id].name : NULL;

    renderer_ui_draw_text_centered(renderer_ui, &application_main_game->resource_pack.font_set, tooltip_position, tooltip_item_name ? tooltip_item_name : "none", UI_TEXT_SIZE);

    ////////////////
    /// 3: Hover ///
    ////////////////
    const fvec2_t hover_position = fvec2(width * 0.5f, height - margin - UI_TEXT_SIZE);

    struct tile *hover_tile      = application_main_game->world.player.has_target_destroy ? world_get_tile(&application_main_game->world, application_main_game->world.player.target_destroy) : NULL;
    uint8_t      hover_tile_id   = hover_tile ? hover_tile->id : BLOCK_NONE;
    const char  *hover_tile_name = hover_tile_id != BLOCK_NONE ? resource_pack->block_infos[hover_tile_id].name : NULL;

    renderer_ui_draw_text_centered(renderer_ui, &application_main_game->resource_pack.font_set, hover_position, hover_tile_name ? hover_tile_name : "none", UI_TEXT_SIZE);
  }
}

void application_main_game_render(struct application_main_game *application_main_game, int width, int height, struct renderer_world *renderer_world, struct renderer_ui *renderer_ui)
{
  glViewport(0, 0, width, height);
  glClearColor(0.52f, 0.81f, 0.98f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  renderer_world_render(renderer_world, width, height, &application_main_game->world, &application_main_game->resource_pack);
  application_main_game_render_ui(application_main_game, width, height, renderer_ui);
}

