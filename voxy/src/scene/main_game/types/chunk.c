#include <voxy/scene/main_game/types/chunk.h>
#include <voxy/scene/main_game/update/light.h>

#include <voxy/core/log.h>

#include <voxy/core/time.h>

#include <assert.h>
#include <stdlib.h>

#define SAVE_INTERVAL 60.0f

static void entities_destroy(struct entities *entities)
{
  for(size_t i=0; i<entities->item_count; ++i)
  {
    struct entity *entity = &entities->items[i];
    const struct entity_info *entity_info = query_entity_info(entity->id);
    if(entity_info->on_dispose)
      entity_info->on_dispose(entity);
  }
  DYNAMIC_ARRAY_CLEAR(*entities);
}

void chunk_destroy(struct chunk *chunk)
{
  if(chunk->left)   chunk->left->right = NULL;
  if(chunk->right)  chunk->right->left = NULL;
  if(chunk->back)   chunk->back->front = NULL;
  if(chunk->front)  chunk->front->back = NULL;
  if(chunk->bottom) chunk->bottom->top = NULL;
  if(chunk->top)    chunk->top->bottom = NULL;

  entities_destroy(&chunk->entities);
  entities_destroy(&chunk->new_entities);
  free(chunk);
}

bool chunk_is_dirty(const struct chunk *chunk)
{
  return *(bool *)&chunk->dirty;
}

bool chunk_should_save(const struct chunk *chunk)
{
  return *(bool *)&chunk->dirty && get_time() >= chunk->last_save_time + SAVE_INTERVAL;
}

void chunk_invalidate_mesh(struct chunk *chunk)
{
  // FIXME: We are only the 6 neighbouring chunks. This is not sufficient if we
  //        also take into account our ambient occlusion calculation.

  atomic_store_explicit(&chunk->remesh, true, memory_order_relaxed);

  if(chunk->left)
    atomic_store_explicit(&chunk->left->remesh, true, memory_order_relaxed);

  if(chunk->back)
    atomic_store_explicit(&chunk->back->remesh, true, memory_order_relaxed);

  if(chunk->bottom)
    atomic_store_explicit(&chunk->bottom->remesh, true, memory_order_relaxed);

  if(chunk->right)
    atomic_store_explicit(&chunk->right->remesh, true, memory_order_relaxed);

  if(chunk->front)
    atomic_store_explicit(&chunk->front->remesh, true, memory_order_relaxed);

  if(chunk->top)
    atomic_store_explicit(&chunk->top->remesh, true, memory_order_relaxed);
}

void chunk_invalidate_mesh_at(struct chunk *chunk, ivec3_t position)
{
  // FIXME: We are only invalidating chunks where blocks at position and the 6
  //        adjacent blocks are located in. This is not sufficient if we also
  //        take into account our ambient occlusion calculation.

  atomic_store_explicit(&chunk->remesh, true, memory_order_relaxed);

  if(position.x == 0 && chunk->left)
    atomic_store_explicit(&chunk->left->remesh, true, memory_order_relaxed);

  if(position.y == 0 && chunk->back)
    atomic_store_explicit(&chunk->back->remesh, true, memory_order_relaxed);

  if(position.z == 0 && chunk->bottom)
    atomic_store_explicit(&chunk->bottom->remesh, true, memory_order_relaxed);

  if(position.x == CHUNK_WIDTH - 1 && chunk->right)
    atomic_store_explicit(&chunk->right->remesh, true, memory_order_relaxed);

  if(position.y == CHUNK_WIDTH - 1 && chunk->front)
    atomic_store_explicit(&chunk->front->remesh, true, memory_order_relaxed);

  if(position.z == CHUNK_WIDTH - 1 && chunk->top)
    atomic_store_explicit(&chunk->top->remesh, true, memory_order_relaxed);
}

void chunk_invalidate_data(struct chunk *chunk)
{
  atomic_store_explicit(&chunk->dirty, true, memory_order_relaxed);
}

void chunk_invalidate_data_at(struct chunk *chunk, ivec3_t position)
{
  (void)position;
  chunk_invalidate_data(chunk);
}

static inline void check_position(ivec3_t position)
{
  assert(0 <= position.x && position.x < CHUNK_WIDTH);
  assert(0 <= position.y && position.y < CHUNK_WIDTH);
  assert(0 <= position.z && position.z < CHUNK_WIDTH);
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

block_id_t chunk_get_block_id(const struct chunk *chunk, ivec3_t position)
{
  check_position(position);
  return chunk->block_ids[position.z][position.y][position.x];
}

block_id_t chunk_get_block_id_ex(const struct chunk *chunk, ivec3_t position)
{
  chunk_traverse(&chunk, &position);
  return chunk ? chunk_get_block_id(chunk, position) : BLOCK_NONE;
}

void chunk_set_block_id_raw(struct chunk *chunk, ivec3_t position, block_id_t id)
{
  check_position(position);
  chunk->block_ids[position.z][position.y][position.x] = id;
}

void chunk_set_block_id(struct chunk *chunk, ivec3_t position, block_id_t id)
{
  chunk_set_block_id_raw(chunk, position, id);
  chunk_invalidate_data_at(chunk, position);
  chunk_invalidate_mesh_at(chunk, position);
}

unsigned chunk_get_block_light_level(const struct chunk *chunk, ivec3_t position)
{
  check_position(position);

  const size_t offset = position.z * CHUNK_WIDTH * CHUNK_WIDTH + position.y * CHUNK_WIDTH + position.x;
  const size_t q = offset / (CHAR_BIT / 4);
  const size_t r = offset % (CHAR_BIT / 4);
  return (chunk->block_light_levels[q] >> (r * 4)) & ((1 << 4) - 1);
}

unsigned chunk_get_block_light_level_ex(const struct chunk *chunk, ivec3_t position)
{
  chunk_traverse(&chunk, &position);
  return chunk ? chunk_get_block_light_level(chunk, position) : -1;
}

void chunk_set_block_light_level_raw(struct chunk *chunk, ivec3_t position, unsigned light_level)
{
  check_position(position);

  const size_t offset = position.z * CHUNK_WIDTH * CHUNK_WIDTH + position.y * CHUNK_WIDTH + position.x;
  const size_t q = offset / (CHAR_BIT / 4);
  const size_t r = offset % (CHAR_BIT / 4);
  chunk->block_light_levels[q] &= ~(((1 << 4) - 1) << (r * 4));
  chunk->block_light_levels[q] |= light_level << (r * 4);
}

void chunk_set_block_light_level(struct chunk *chunk, ivec3_t position, unsigned light_level)
{
  chunk_set_block_light_level_raw(chunk, position, light_level);
  chunk_invalidate_data_at(chunk, position);
  chunk_invalidate_mesh_at(chunk, position);
}

void chunk_get_block_light_level_atomic(const struct chunk *chunk, ivec3_t position, unsigned *light_level, unsigned char *tmp)
{
  check_position(position);

  const size_t offset = position.z * CHUNK_WIDTH * CHUNK_WIDTH + position.y * CHUNK_WIDTH + position.x;
  const size_t q = offset / (CHAR_BIT / 4);
  const size_t r = offset % (CHAR_BIT / 4);

  *tmp = atomic_load((_Atomic unsigned char *)&chunk->block_light_levels[q]);
  *light_level = (*tmp >> (r * 4)) & ((1 << 4) - 1);
}

bool chunk_set_block_light_level_atomic(struct chunk *chunk, ivec3_t position, unsigned *light_level, unsigned char *tmp)
{
  check_position(position);

  const size_t offset = position.z * CHUNK_WIDTH * CHUNK_WIDTH + position.y * CHUNK_WIDTH + position.x;
  const size_t q = offset / (CHAR_BIT / 4);
  const size_t r = offset % (CHAR_BIT / 4);

  unsigned char desired = *tmp;
  desired &= ~(((1 << 4) - 1) << (r * 4));
  desired |= *light_level << (r * 4);
  if(!atomic_compare_exchange_strong((_Atomic unsigned char *)&chunk->block_light_levels[q], tmp, desired))
  {
    *light_level = (*tmp >> (r * 4)) & ((1 << 4) - 1);
    return false;
  }

  chunk_invalidate_data_at(chunk, position);
  chunk_invalidate_mesh_at(chunk, position);
  return true;
}

void chunk_set_block_raw(struct chunk *chunk, ivec3_t position, block_id_t id)
{
  const struct block_info *info = query_block_info(id);
  chunk_set_block_id_raw(chunk, position, id);
  chunk_set_block_light_level_raw(chunk, position, info->light_level);
}

void chunk_set_block(struct chunk *chunk, ivec3_t position, block_id_t id)
{
  const unsigned old_block_light_level = chunk_get_block_light_level(chunk, position);

  const struct block_info *info = query_block_info(id);
  chunk_set_block_id(chunk, position, id);
  chunk_set_block_light_level(chunk, position, info->light_level);

  const unsigned new_block_light_level = chunk_get_block_light_level(chunk, position);

  if(old_block_light_level < new_block_light_level)
    enqueue_light_create_update(chunk, position.x, position.y, position.z);

  // FIXME: Light destroy update really only support the case if light level
  //        changes to 0. This happens to be the only possible cases for now.
  if(old_block_light_level >= new_block_light_level)
    enqueue_light_destroy_update(chunk, position.x, position.y, position.z, old_block_light_level);
}

void chunk_add_entity_raw(struct chunk *chunk, struct entity entity)
{
  DYNAMIC_ARRAY_APPEND(chunk->entities, entity);
}

void chunk_add_entity(struct chunk *chunk, struct entity entity)
{
  DYNAMIC_ARRAY_APPEND(chunk->new_entities, entity);
}

void chunk_commit_add_entities(struct chunk *chunk)
{
  DYNAMIC_ARRAY_APPEND_MANY(chunk->entities, chunk->new_entities.items, chunk->new_entities.item_count);
  DYNAMIC_ARRAY_CLEAR(chunk->new_entities);
}

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX chunk
#define SC_HASH_TABLE_NODE_TYPE struct chunk
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

ivec3_t chunk_key(struct chunk *chunk) { return chunk->position; }
size_t chunk_hash(ivec3_t position) { return ivec3_hash(position); }
int chunk_compare(ivec3_t position1, ivec3_t position2) { return ivec3_compare(position1, position2); }
void chunk_dispose(struct chunk *chunk) { chunk_destroy(chunk); }

