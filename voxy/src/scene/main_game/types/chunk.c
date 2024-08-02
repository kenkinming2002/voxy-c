#include <voxy/scene/main_game/types/chunk.h>
#include <voxy/scene/main_game/update/light.h>

#include <voxy/core/log.h>

bool chunk_is_dirty(const struct chunk *chunk)
{
  return chunk_data_is_dirty(chunk->data);
}

static void chunk_traverse(const struct chunk **chunk, ivec3_t *position)
{
  while(*chunk)
  {
    if(position->x < 0)
    {
      *chunk = (*chunk)->left;
      position->x += CHUNK_WIDTH;
    }
    else if(position->y < 0)
    {
      *chunk = (*chunk)->back;
      position->y += CHUNK_WIDTH;
    }
    else if(position->z < 0)
    {
      *chunk = (*chunk)->bottom;
      position->z += CHUNK_WIDTH;
    }
    else if(position->x >= CHUNK_WIDTH)
    {
      *chunk = (*chunk)->right;
      position->x -= CHUNK_WIDTH;
    }
    else if(position->y >= CHUNK_WIDTH)
    {
      *chunk = (*chunk)->front;
      position->y -= CHUNK_WIDTH;
    }
    else if(position->z >= CHUNK_WIDTH)
    {
      *chunk = (*chunk)->top;
      position->z -= CHUNK_WIDTH;
    }
    else
      return;
  }
}

void chunk_invalidate_mesh_at(struct chunk *chunk, ivec3_t position)
{
  // FIXME: We are only invalidating chunks where blocks at position and the 6
  //        adjacent blocks are located in. This is not sufficient if we also
  //        take into account our ambient occlusion calculation.

  chunk->mesh_invalidated = true;

  if(position.x == 0 && chunk->left)
    chunk->left->mesh_invalidated = true;

  if(position.y == 0 && chunk->back)
    chunk->back->mesh_invalidated = true;

  if(position.z == 0 && chunk->bottom)
    chunk->bottom->mesh_invalidated = true;

  if(position.x == CHUNK_WIDTH - 1 && chunk->right)
    chunk->right->mesh_invalidated = true;

  if(position.y == CHUNK_WIDTH - 1 && chunk->front)
    chunk->front->mesh_invalidated = true;

  if(position.z == CHUNK_WIDTH - 1 && chunk->top)
    chunk->top->mesh_invalidated = true;
}

block_id_t chunk_get_block_id(const struct chunk *chunk, ivec3_t position)
{
  return chunk_data_get_block_id(chunk->data, position);
}

block_id_t chunk_get_block_id_ex(const struct chunk *chunk, ivec3_t position)
{
  chunk_traverse(&chunk, &position);
  if(chunk && chunk->data)
    return chunk_data_get_block_id(chunk->data, position);
  else
    return BLOCK_NONE;
}

void chunk_set_block_id(struct chunk *chunk, ivec3_t position, block_id_t id)
{
  chunk_data_set_block_id(chunk->data, position, id);
  chunk_invalidate_mesh_at(chunk, position);
}

unsigned chunk_get_block_light_level(const struct chunk *chunk, ivec3_t position)
{
  return chunk_data_get_block_light_level(chunk->data, position);
}

unsigned chunk_get_block_light_level_ex(const struct chunk *chunk, ivec3_t position)
{
  chunk_traverse(&chunk, &position);
  if(chunk && chunk->data)
    return chunk_data_get_block_light_level(chunk->data, position);
  else
    return -1;
}

void chunk_set_block_light_level(struct chunk *chunk, ivec3_t position, unsigned light_level)
{
  chunk_data_set_block_light_level(chunk->data, position, light_level);
  chunk_invalidate_mesh_at(chunk, position);
}

void chunk_get_block_light_level_atomic(const struct chunk *chunk, ivec3_t position, unsigned *light_level, unsigned char *tmp)
{
  chunk_data_get_block_light_level_atomic(chunk->data, position, light_level, tmp);
}

bool chunk_set_block_light_level_atomic(struct chunk *chunk, ivec3_t position, unsigned *light_level, unsigned char *tmp)
{
  if(chunk_data_set_block_light_level_atomic(chunk->data, position, light_level, tmp))
  {
    chunk_invalidate_mesh_at(chunk, position);
    return true;
  }
  else
    return false;
}

void chunk_set_block(struct chunk *chunk, ivec3_t position, block_id_t id)
{
  const unsigned old_block_light_level = chunk_data_get_block_light_level(chunk->data, position);

  chunk_data_set_block(chunk->data, position, id);
  chunk_invalidate_mesh_at(chunk, position);

  const unsigned new_block_light_level = chunk_data_get_block_light_level(chunk->data, position);

  if(old_block_light_level < new_block_light_level)
    enqueue_light_create_update(chunk, position.x, position.y, position.z);

  // FIXME: Light destroy update really only support the case if light level
  //        changes to 0. This happens to be the only possible cases for now.
  if(old_block_light_level >= new_block_light_level)
    enqueue_light_destroy_update(chunk, position.x, position.y, position.z, old_block_light_level);
}

void chunk_add_entity_raw(struct chunk *chunk, struct entity entity)
{
  chunk_data_add_entity_raw(chunk->data, entity);
}

void chunk_add_entity(struct chunk *chunk, struct entity entity)
{
  chunk_data_add_entity(chunk->data, entity);
}

void chunk_commit_add_entities(struct chunk *chunk)
{
  chunk_data_commit_add_entities(chunk->data);
}
