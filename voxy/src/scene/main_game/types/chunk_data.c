#include <voxy/scene/main_game/types/chunk_data.h>
#include <voxy/scene/main_game/types/registry.h>

#include <assert.h>
#include <stdlib.h>

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

void chunk_data_destroy(struct chunk_data *chunk_data)
{
  entities_destroy(&chunk_data->entities);
  entities_destroy(&chunk_data->new_entities);
  free(chunk_data);
}

bool chunk_data_is_dirty(const struct chunk_data *chunk_data)
{
  return chunk_data->dirty;
}

static inline void check_position(ivec3_t position)
{
  assert(0 <= position.x && position.x < CHUNK_WIDTH);
  assert(0 <= position.y && position.y < CHUNK_WIDTH);
  assert(0 <= position.z && position.z < CHUNK_WIDTH);
}

block_id_t chunk_data_get_block_id(const struct chunk_data *chunk_data, ivec3_t position)
{
  check_position(position);
  return chunk_data->block_ids[position.z][position.y][position.x];
}

void chunk_data_set_block_id(struct chunk_data *chunk_data, ivec3_t position, block_id_t id)
{
  check_position(position);
  chunk_data->dirty = true;
  chunk_data->block_ids[position.z][position.y][position.x] = id;
}

static unsigned chunk_data_get_bits(const struct chunk_data *chunk_data, const unsigned char data[], ivec3_t position, unsigned width)
{
  check_position(position);
  (void)chunk_data;

  const size_t offset = position.z * CHUNK_WIDTH * CHUNK_WIDTH + position.y * CHUNK_WIDTH + position.x;
  const size_t q = offset / (CHAR_BIT / width);
  const size_t r = offset % (CHAR_BIT / width);
  return (data[q] >> (r * width)) & ((1 << width) - 1);
}

static void chunk_data_set_bits(struct chunk_data *chunk_data, unsigned char data[], ivec3_t position, unsigned width, unsigned bits)
{
  check_position(position);
  chunk_data->dirty = true;

  const size_t offset = position.z * CHUNK_WIDTH * CHUNK_WIDTH + position.y * CHUNK_WIDTH + position.x;
  const size_t q = offset / (CHAR_BIT / width);
  const size_t r = offset % (CHAR_BIT / width);
  data[q] &= ~(((1 << width) - 1) << (r * width));
  data[q] |= bits << (r * width);
}

unsigned chunk_data_get_block_ether(const struct chunk_data *chunk_data, ivec3_t position)
{
  return chunk_data_get_bits(chunk_data, chunk_data->block_ethers, position, 1);
}

void chunk_data_set_block_ether(struct chunk_data *chunk_data, ivec3_t position, unsigned ether)
{
  return chunk_data_set_bits(chunk_data, chunk_data->block_ethers, position, 1, ether);
}

unsigned chunk_data_get_block_light_level(const struct chunk_data *chunk_data, ivec3_t position)
{
  return chunk_data_get_bits(chunk_data, chunk_data->block_light_levels, position, 4);
}

void chunk_data_set_block_light_level(struct chunk_data *chunk_data, ivec3_t position, unsigned light_level)
{
  return chunk_data_set_bits(chunk_data, chunk_data->block_light_levels, position, 4, light_level);
}

void chunk_data_set_block(struct chunk_data *chunk_data, ivec3_t position, block_id_t id)
{
  const struct block_info *info = query_block_info(id);
  chunk_data_set_block_id(chunk_data, position, id);
  chunk_data_set_block_ether(chunk_data, position, info->ether);
  chunk_data_set_block_light_level(chunk_data, position, info->light_level);
}

void chunk_data_add_entity_raw(struct chunk_data *chunk_data, struct entity entity)
{
  DYNAMIC_ARRAY_APPEND(chunk_data->entities, entity);
}

void chunk_data_add_entity(struct chunk_data *chunk_data, struct entity entity)
{
  DYNAMIC_ARRAY_APPEND(chunk_data->new_entities, entity);
}

void chunk_data_commit_add_entities(struct chunk_data *chunk_data)
{
  DYNAMIC_ARRAY_APPEND_MANY(chunk_data->entities, chunk_data->new_entities.items, chunk_data->new_entities.item_count);
  DYNAMIC_ARRAY_CLEAR(chunk_data->new_entities);
}

