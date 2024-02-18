#include <voxy/main_game/main_game.h>

#include <voxy/main_game/chunk_generate.h>
#include <voxy/main_game/light.h>
#include <voxy/main_game/physics.h>
#include <voxy/main_game/chunk_remesh.h>
#include <voxy/main_game/world_render.h>

#include <voxy/main_game/world.h>
#include <voxy/main_game/player.h>

#include <voxy/graphics/ui.h>

#include <voxy/core/window.h>

#include <glad/glad.h>

#include <stdio.h>
#include <stdlib.h>

static float randf(float low, float high)
{
  return low + (float)rand() / (float)RAND_MAX * (high - low);
}

static void weird_entity_update(struct entity *entity, float dt)
{
  entity_jump(entity, 20.0f);
  entity_move(entity, fvec2(randf(-1.0f, 1.0f), randf(-1.0f, 1.0f)), 30.0f, dt);
}

void main_game_update(float dt)
{
  ui_reset();

  // 0: Entity Generation? This should be temporary!??
  static float cooldown;
  cooldown += dt;
  if(cooldown >= 2.0f)
  {
    cooldown -= 2.0f;

    struct player *player = player_get();
    if(player)
    {
      struct entity *player_entity = player_as_entity(player);
      struct entity *entity        = malloc(sizeof *entity);
      *entity = *player_entity;
      entity->update = &weird_entity_update;
      world_entity_add(entity);
    }
  }

  // 1: Update World
  update_chunk_generate();
  update_spawn_player();

  for(size_t i=0; i<entity_count; ++i)
    if(entities[i]->update)
      entities[i]->update(entities[i], dt);

  update_light();
  update_physics(dt);
  update_chunk_remesh();
}

void main_game_render()
{
  glViewport(0, 0, window_size.x, window_size.y);
  world_render();
  ui_render();
}

