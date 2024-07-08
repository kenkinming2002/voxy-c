#include <voxy/scene/main_game/types/chunk.h>

block_id_t chunk_get_block_id(struct chunk *chunk, int x, int y, int z)
{
  return chunk->data->blocks[z][y][x].id;
}

unsigned chunk_get_light_level(struct chunk *chunk, int x, int y, int z)
{
  return chunk->data->blocks[z][y][x].light_level;
}

unsigned chunk_get_block_ether(struct chunk *chunk, int x, int y, int z)
{
  return chunk->data->blocks[z][y][x].ether;
}

void chunk_set_block_id(struct chunk *chunk, int x, int y, int z, block_id_t id)
{
  if(chunk->data->blocks[z][y][x].id != id)
  {
    chunk->data->blocks[z][y][x].id = id;
    chunk->mesh_invalidated = true;

    if(x == 0 && chunk->left)
      chunk->left->mesh_invalidated = true;

    if(y == 0 && chunk->back)
      chunk->back->mesh_invalidated = true;

    if(z == 0 && chunk->bottom)
      chunk->bottom->mesh_invalidated = true;

    if(x == CHUNK_WIDTH - 1 && chunk->right)
      chunk->right->mesh_invalidated = true;

    if(y == CHUNK_WIDTH - 1 && chunk->front)
      chunk->front->mesh_invalidated = true;

    if(z == CHUNK_WIDTH - 1 && chunk->top)
      chunk->top->mesh_invalidated = true;
  }
}

void chunk_set_block_light_level(struct chunk *chunk, int x, int y, int z, unsigned light_level)
{
  if(chunk->data->blocks[z][y][x].light_level != light_level)
  {
    chunk->data->blocks[z][y][x].light_level = light_level;
    chunk->mesh_invalidated = true;

    if(x == 0 && chunk->left)
      chunk->left->mesh_invalidated = true;

    if(y == 0 && chunk->back)
      chunk->back->mesh_invalidated = true;

    if(z == 0 && chunk->bottom)
      chunk->bottom->mesh_invalidated = true;

    if(x == CHUNK_WIDTH - 1 && chunk->right)
      chunk->right->mesh_invalidated = true;

    if(y == CHUNK_WIDTH - 1 && chunk->front)
      chunk->front->mesh_invalidated = true;

    if(z == CHUNK_WIDTH - 1 && chunk->top)
      chunk->top->mesh_invalidated = true;
  }
}

void chunk_set_block_ether(struct chunk *chunk, int x, int y, int z, unsigned ether)
{
  if(chunk->data->blocks[z][y][x].ether != ether)
  {
    chunk->data->blocks[z][y][x].ether = ether;
    chunk->mesh_invalidated = true;

    if(x == 0 && chunk->left)
      chunk->left->mesh_invalidated = true;

    if(y == 0 && chunk->back)
      chunk->back->mesh_invalidated = true;

    if(z == 0 && chunk->bottom)
      chunk->bottom->mesh_invalidated = true;

    if(x == CHUNK_WIDTH - 1 && chunk->right)
      chunk->right->mesh_invalidated = true;

    if(y == CHUNK_WIDTH - 1 && chunk->front)
      chunk->front->mesh_invalidated = true;

    if(z == CHUNK_WIDTH - 1 && chunk->top)
      chunk->top->mesh_invalidated = true;
  }
}

void chunk_set_block(struct chunk *chunk, int x, int y, int z, block_id_t id)
{
  const struct block_info *info = query_block_info(id);
  chunk_set_block_id(chunk, x, y, z, id);
  chunk_set_block_light_level(chunk, x, y, z, info->light_level);
  chunk_set_block_ether(chunk, x, y, z, info->ether);
}

struct block *chunk_get_block(struct chunk *chunk, ivec3_t position)
{
  if(position.z < 0) return chunk->bottom ? chunk_get_block(chunk->bottom, ivec3_add(position, ivec3(0, 0, CHUNK_WIDTH))) : NULL;
  if(position.y < 0) return chunk->back   ? chunk_get_block(chunk->back,   ivec3_add(position, ivec3(0, CHUNK_WIDTH, 0))) : NULL;
  if(position.x < 0) return chunk->left   ? chunk_get_block(chunk->left,   ivec3_add(position, ivec3(CHUNK_WIDTH, 0, 0))) : NULL;

  if(position.z >= CHUNK_WIDTH) return chunk->top   ? chunk_get_block(chunk->top,   ivec3_sub(position, ivec3(0, 0, CHUNK_WIDTH))) : NULL;
  if(position.y >= CHUNK_WIDTH) return chunk->front ? chunk_get_block(chunk->front, ivec3_sub(position, ivec3(0, CHUNK_WIDTH, 0))) : NULL;
  if(position.x >= CHUNK_WIDTH) return chunk->right ? chunk_get_block(chunk->right, ivec3_sub(position, ivec3(CHUNK_WIDTH, 0, 0))) : NULL;

  return chunk->data ? chunk_data_get_block(chunk->data, position) : NULL;
}

struct entity *chunk_add_entity(struct chunk *chunk, struct entity entity)
{
  return chunk->data ? chunk_data_add_entity(chunk->data, entity) : NULL;
}
