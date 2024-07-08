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

bool cursor_move(struct cursor *cursor, enum direction direction)
{
  ivec3_t position = ivec3(cursor->x, cursor->y, cursor->z);
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

struct block *cursor_get(struct cursor cursor)
{
  return &cursor.chunk->data->blocks[cursor.z][cursor.y][cursor.x];
}

