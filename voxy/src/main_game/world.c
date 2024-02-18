#include <voxy/main_game/world.h>

#include <voxy/main_game/config.h>

#include <stdlib.h>

struct chunk_hash_table chunks;

struct chunk *chunks_invalidated_light_head;
struct chunk *chunks_invalidated_light_tail;

struct chunk *chunks_invalidated_mesh_head;
struct chunk *chunks_invalidated_mesh_tail;

struct chunk *world_chunk_lookup(ivec3_t position)
{
  return chunk_hash_table_lookup(&chunks, position);
}

struct chunk *world_chunk_create(ivec3_t position)
{
  struct chunk *chunk = malloc(sizeof *chunk);

  chunk->position = position;
  chunk_hash_table_insert_unchecked(&chunks, chunk);

  chunk->left   = world_chunk_lookup(ivec3_add(chunk->position, ivec3(-1,  0,  0)));
  chunk->right  = world_chunk_lookup(ivec3_add(chunk->position, ivec3( 1,  0,  0)));
  chunk->back   = world_chunk_lookup(ivec3_add(chunk->position, ivec3( 0, -1,  0)));
  chunk->front  = world_chunk_lookup(ivec3_add(chunk->position, ivec3( 0,  1,  0)));
  chunk->bottom = world_chunk_lookup(ivec3_add(chunk->position, ivec3( 0,  0, -1)));
  chunk->top    = world_chunk_lookup(ivec3_add(chunk->position, ivec3( 0,  0,  1)));

  if(chunk->left)   chunk->left->right = chunk;
  if(chunk->right)  chunk->right->left = chunk;
  if(chunk->back)   chunk->back->front = chunk;
  if(chunk->front)  chunk->front->back = chunk;
  if(chunk->bottom) chunk->bottom->top = chunk;
  if(chunk->top)    chunk->top->bottom = chunk;

  chunk->mesh_invalidated = false;
  chunk->mesh_next        = NULL;

  chunk->light_invalidated = false;
  chunk->light_next        = NULL;

  glGenVertexArrays(1, &chunk->vao_opaque);
  glGenBuffers(1, &chunk->vbo_opaque);
  chunk->count_opaque = 0;

  glGenVertexArrays(1, &chunk->vao_transparent);
  glGenBuffers(1, &chunk->vbo_transparent);
  chunk->count_transparent = 0;

  return chunk;
}

void world_chunk_invalidate_light(struct chunk *chunk)
{
  if(chunk && !chunk->light_invalidated)
  {
    chunk->light_invalidated = true;
    chunk->light_next = NULL;
    if(chunks_invalidated_light_tail)
    {
      chunks_invalidated_light_tail->light_next = chunk;
      chunks_invalidated_light_tail = chunk;
    }
    else
    {
      chunks_invalidated_light_head = chunk;
      chunks_invalidated_light_tail = chunk;
    }
  }
}

void world_chunk_invalidate_mesh(struct chunk *chunk)
{
  if(chunk && !chunk->mesh_invalidated)
  {
    chunk->mesh_invalidated = true;
    chunk->mesh_next = NULL;
    if(chunks_invalidated_mesh_tail)
    {
      chunks_invalidated_mesh_tail->mesh_next = chunk;
      chunks_invalidated_mesh_tail = chunk;
    }
    else
    {
      chunks_invalidated_mesh_head = chunk;
      chunks_invalidated_mesh_tail = chunk;
    }
  }
}

static inline void split_position(ivec3_t position, ivec3_t *chunk_position, ivec3_t *block_position)
{
  for(int i=0; i<3; ++i)
  {
    (*block_position).values[i] = ((position.values[i] % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH;
    (*chunk_position).values[i] = (position.values[i] - (*block_position).values[i]) / CHUNK_WIDTH;
  }
}

struct block *world_block_get(ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);

  struct chunk *chunk = world_chunk_lookup(chunk_position);
  if(!chunk)
    return NULL;

  return &chunk->blocks[block_position.z][block_position.y][block_position.x];
}

void world_block_set(ivec3_t position, uint8_t block_id)
{
  struct block *block = world_block_get(position);
  if(!block)
    return;

  block->id = block_id;

  world_block_invalidate_light(position);
  world_block_invalidate_mesh(position);
  world_block_invalidate_mesh(ivec3_add(position, ivec3(-1, 0, 0)));
  world_block_invalidate_mesh(ivec3_add(position, ivec3( 1, 0, 0)));
  world_block_invalidate_mesh(ivec3_add(position, ivec3(0, -1, 0)));
  world_block_invalidate_mesh(ivec3_add(position, ivec3(0,  1, 0)));
  world_block_invalidate_mesh(ivec3_add(position, ivec3(0, 0, -1)));
  world_block_invalidate_mesh(ivec3_add(position, ivec3(0, 0,  1)));
}

void world_block_invalidate_light(ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);
  world_chunk_invalidate_light(world_chunk_lookup(chunk_position));
}

void world_block_invalidate_mesh(ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);
  world_chunk_invalidate_mesh(world_chunk_lookup(chunk_position));
}

struct entity **entities;
size_t          entity_count;
size_t          entity_capacity;

void world_entity_add(struct entity *entity)
{
  if(entity_capacity == entity_count)
  {
    entity_capacity = entity_capacity != 0 ? entity_capacity * 2 : 1;
    entities        = realloc(entities, entity_capacity * sizeof *entities);
  }
  entities[entity_count++] = entity;
}

