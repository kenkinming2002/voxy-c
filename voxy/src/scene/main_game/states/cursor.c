#include <voxy/scene/main_game/states/cursor.h>
#include <voxy/scene/main_game/states/chunks.h>

#include <voxy/scene/main_game/types/chunk.h>

static inline void split_position(ivec3_t position, ivec3_t *chunk_position, ivec3_t *block_position)
{
  for(int i=0; i<3; ++i)
  {
    (*block_position).values[i] = ((position.values[i] % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH;
    (*chunk_position).values[i] = (position.values[i] - (*block_position).values[i]) / CHUNK_WIDTH;
  }
}

bool cursor_at(ivec3_t position, struct cursor *cursor)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);

  cursor->chunk = world_get_chunk(chunk_position);
  cursor->x = block_position.x;
  cursor->y = block_position.y;
  cursor->z = block_position.z;
  return cursor->chunk && cursor->chunk->data;
}

ivec3_t cursor_get_chunk_position(struct cursor cursor)
{
  return cursor.chunk->position;
}

ivec3_t cursor_get_local_position(struct cursor cursor)
{
  return ivec3(cursor.x, cursor.y, cursor.z);
}

ivec3_t cursor_get_global_position(struct cursor cursor)
{
  return local_position_to_global_position(cursor_get_local_position(cursor), cursor_get_chunk_position(cursor));
}

bool cursor_move(struct cursor *cursor, direction_t direction)
{
  ivec3_t position = cursor_get_local_position(*cursor);
  ivec3_t new_position = ivec3_add(position, direction_as_ivec(direction));

  switch(direction)
  {
  case DIRECTION_LEFT:
    if(new_position.x < 0)
    {
      cursor->chunk = cursor->chunk->left;
      new_position.x += CHUNK_WIDTH;
    }
    break;
  case DIRECTION_BACK:
    if(new_position.y < 0)
    {
      cursor->chunk = cursor->chunk->back;
      new_position.y += CHUNK_WIDTH;
    }
    break;
  case DIRECTION_BOTTOM:
    if(new_position.z < 0)
    {
      cursor->chunk = cursor->chunk->bottom;
      new_position.z += CHUNK_WIDTH;
    }
    break;
  case DIRECTION_RIGHT:
    if(new_position.x >= CHUNK_WIDTH)
    {
      cursor->chunk = cursor->chunk->right;
      new_position.x -= CHUNK_WIDTH;
    }
    break;
  case DIRECTION_FRONT:
    if(new_position.y >= CHUNK_WIDTH)
    {
      cursor->chunk = cursor->chunk->front;
      new_position.y -= CHUNK_WIDTH;
    }
    break;
  case DIRECTION_TOP:
    if(new_position.z >= CHUNK_WIDTH)
    {
      cursor->chunk = cursor->chunk->top;
      new_position.z -= CHUNK_WIDTH;
    }
    break;
  case DIRECTION_COUNT:
      assert(0 && "Unreachable");
  }

  cursor->x = new_position.x;
  cursor->y = new_position.y;
  cursor->z = new_position.z;
  return cursor->chunk && cursor->chunk->data;
}

block_id_t cursor_get_block_id(struct cursor cursor)
{
  return chunk_get_block_id(cursor.chunk, cursor_get_local_position(cursor));
}

void cursor_set_block_id(struct cursor cursor, block_id_t id)
{
  chunk_set_block_id(cursor.chunk, cursor_get_local_position(cursor), id);
}

unsigned cursor_get_block_light_level(struct cursor cursor)
{
  return chunk_get_block_light_level(cursor.chunk, cursor_get_local_position(cursor));
}

void cursor_set_block_light_level(struct cursor cursor, unsigned light_level)
{
  chunk_set_block_light_level(cursor.chunk, cursor_get_local_position(cursor), light_level);
}

void cursor_get_block_light_level_atomic(struct cursor cursor, unsigned *light_level, unsigned char *tmp)
{
  chunk_get_block_light_level_atomic(cursor.chunk, cursor_get_local_position(cursor), light_level, tmp);
}

bool cursor_set_block_light_level_atomic(struct cursor cursor, unsigned *light_level, unsigned char *tmp)
{
  return chunk_set_block_light_level_atomic(cursor.chunk, cursor_get_local_position(cursor), light_level, tmp);
}
