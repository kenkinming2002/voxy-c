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

void application_main_game_render(struct application_main_game *application_main_game, int width, int height, struct renderer_world *renderer_world, struct renderer_ui *renderer_ui)
{
  glViewport(0, 0, width, height);
  glClearColor(0.52f, 0.81f, 0.98f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  renderer_world_render(renderer_world, width, height, &application_main_game->world, &application_main_game->resource_pack);
  renderer_ui_begin(renderer_ui, fvec2(width, height));

  char buffer[256];

  // 1: Hotbar
  {
    const int count = 9;

    const float sep               = minf(width, height) * 0.006f;
    const float inner_width       = minf(width, height) * 0.05f;
    const float outer_width       = inner_width + 2.0f * sep;
    const float total_width       = count * inner_width + (count + 1) * sep;
    const float total_height      = outer_width;
    const float margin_horizontal = (width - total_width) * 0.5f;
    const float margin_vertical   = height * 0.03f;

    snprintf(buffer, sizeof buffer, "Selected %d 你好 😀", application_main_game->world.player.selection);
    renderer_ui_draw_text_centered(renderer_ui, &application_main_game->resource_pack.font_set, fvec2(width * 0.5f, margin_vertical + outer_width + sep), buffer, 24);
    renderer_ui_draw_quad_rounded(renderer_ui, fvec2(margin_horizontal, margin_vertical), fvec2(total_width, total_height), sep, fvec4(0.9f, 0.9f, 0.9f, 0.3f));
    for(int i=0; i<count; ++i)
    {
      fvec4_t color = i + 1 == application_main_game->world.player.selection ? fvec4(0.95f, 0.75f, 0.75f, 0.8f) : fvec4(0.95f, 0.95f, 0.95f, 0.7f);
      renderer_ui_draw_quad_rounded(renderer_ui, fvec2(margin_horizontal + i * inner_width + (i + 1) * sep, margin_vertical + sep), fvec2(inner_width, inner_width), sep, color);
    }
  }

  // 2: Block name
  {
    const float margin_vertical = height * 0.03f;

    struct tile *tile;
    if(application_main_game->world.player.has_target_destroy && (tile = world_get_tile(&application_main_game->world, application_main_game->world.player.target_destroy)))
    {
      const char *name = application_main_game->resource_pack.block_infos[tile->id].name;
      renderer_ui_draw_text_centered(renderer_ui, &application_main_game->resource_pack.font_set, fvec2(width * 0.5f, height - margin_vertical - 24), name, 24);
    }
  }
}

