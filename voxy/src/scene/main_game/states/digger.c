#include <voxy/scene/main_game/states/digger.h>
#include <voxy/scene/main_game/states/chunks.h>

struct digger g_digger = {
  .position = {0},
  .damage = 0.0f,
};

void digger_reset(struct digger *digger)
{
  if(digger->damage != 0.0f)
  {
    world_invalidate_mesh_at(digger->position);
    digger->position = ivec3_zero();
    digger->damage = 0.0f;
  }
}

void digger_set_position(struct digger *digger, ivec3_t position)
{
  if(!ivec3_eql(digger->position, position))
  {
    world_invalidate_mesh_at(digger->position);
    digger->position = position;
    digger->damage = 0.0f;
  }
}

void digger_dig(struct digger *digger, float damage, struct entity *entity)
{
  world_invalidate_mesh_at(digger->position);
  digger->damage += damage;
  if(digger->damage >= 1.0f)
    world_set_block(digger->position, 0, entity); // FIXME: We are hardcoding the fact that empty block have id 0
}
