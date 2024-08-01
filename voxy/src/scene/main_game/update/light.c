#include <voxy/scene/main_game/update/light.h>

#include <voxy/scene/main_game/types/chunk.h>
#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/cursor.h>

#include <voxy/core/log.h>
#include <voxy/dynamic_array.h>
#include <voxy/utils.h>

#include <time.h>

// References:
//  - https://www.seedofandromeda.com/blogs/29-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-1
//  - https://www.seedofandromeda.com/blogs/30-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-2
// The link is dead but the article could be still be accessed via wayback machine.

// We need to store light creation/destruction in a fifo data structure i.e.
// queue but obviously we do not have a implementation in C. We are using the
// singly linked list from BSD. This is both non-portable and inefficien
// considering the size of our structure is so small.
struct light_creation
{
  struct chunk *chunk;
  uint8_t x : 4;
  uint8_t y : 4;
  uint8_t z : 4;
};

// We need to store light creation/destruction in a fifo data structure i.e.
// queue but obviously we do not have a implementation in C. We are using the
// singly linked list from BSD. This is both non-portable and inefficien
// considering the size of our structure is so small.
struct light_destruction
{
  struct chunk *chunk;
  uint8_t x : 4;
  uint8_t y : 4;
  uint8_t z : 4;
  uint8_t light_level : 4;
};

DYNAMIC_ARRAY_DEFINE(light_creations, struct light_creation);
DYNAMIC_ARRAY_DEFINE(light_destructions, struct light_destruction);

static struct cursor light_creation_cursor(struct light_creation light_creation)
{
  struct cursor cursor;
  cursor.chunk = light_creation.chunk;
  cursor.x = light_creation.x;
  cursor.y = light_creation.y;
  cursor.z = light_creation.z;
  return cursor;
}

static struct cursor light_destruction_cursor(struct light_destruction light_destruction)
{
  struct cursor cursor;
  cursor.chunk = light_destruction.chunk;
  cursor.x = light_destruction.x;
  cursor.y = light_destruction.y;
  cursor.z = light_destruction.z;
  return cursor;
}

static struct light_creations light_creations;
static struct light_destructions light_destructions;

void enqueue_light_create_update(struct chunk *chunk, unsigned x, unsigned y, unsigned z)
{
  struct light_creation light_creation;
  light_creation.chunk = chunk;
  light_creation.x = x;
  light_creation.y = y;
  light_creation.z = z;
  DYNAMIC_ARRAY_APPEND(light_creations, light_creation);
}

void enqueue_light_destroy_update(struct chunk *chunk, unsigned x, unsigned y, unsigned z, unsigned light_level)
{
  struct light_destruction light_destruction;
  light_destruction.chunk = chunk;
  light_destruction.x = x;
  light_destruction.y = y;
  light_destruction.z = z;
  light_destruction.light_level = light_level;
  DYNAMIC_ARRAY_APPEND(light_destructions, light_destruction);
}

static void update_light_creation(void)
{
  size_t count = 0;
  clock_t begin = clock();

  struct light_creations curr_light_creations = {0};
  while(light_creations.item_count != 0)
  {
    SWAP(curr_light_creations, light_creations);
    for(size_t i=0; i<curr_light_creations.item_count; ++i)
    {
      const struct light_creation light_creation = curr_light_creations.items[i];

      // Esentially, we want to do is the following:
      //   1. Loop through each adjacent block
      //   2. Check we can propagate light to each of them i.e. it is both
      //     1. Not opaque
      //     2. Have a lower light level strictly lower than what we could propagate
      //   3. Propagate if that is the case
      struct cursor cursor = light_creation_cursor(light_creation);
      for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
      {
        struct cursor neighbour_cursor = light_creation_cursor(light_creation);
        if(cursor_move(&neighbour_cursor, direction))
        {
          const struct block_info *neighbour_block_info = query_block_info(cursor_get_block_id(neighbour_cursor));
          if(neighbour_block_info->type != BLOCK_TYPE_OPAQUE)
          {
            const unsigned light_level = cursor_get_block_light_level(cursor);
            const unsigned neighbour_light_level = cursor_get_block_light_level(neighbour_cursor);

            bool propagate = false;
            unsigned new_neighbour_light_level;

            if(direction == DIRECTION_BOTTOM && light_level == 15 && neighbour_light_level < 15)
            {
              propagate = true;
              new_neighbour_light_level = 15;
            }
            else if(light_level > neighbour_light_level + 1)
            {
              propagate = true;
              new_neighbour_light_level = light_level - 1;
            }

            if(propagate)
            {
              cursor_set_block_light_level(neighbour_cursor, new_neighbour_light_level);
              enqueue_light_create_update(neighbour_cursor.chunk, neighbour_cursor.x, neighbour_cursor.y, neighbour_cursor.z);
            }
          }
        }
      }
    }
    count += curr_light_creations.item_count;
    curr_light_creations.item_count = 0;
  }
  DYNAMIC_ARRAY_CLEAR(light_creations);
  DYNAMIC_ARRAY_CLEAR(curr_light_creations);

  clock_t end = clock();
  if(count != 0)
    LOG_INFO("Light System: Processed %zu creation updates in %fs - Average %fs", count, (float)(end - begin) / (float)CLOCKS_PER_SEC, (float)(end - begin) / (float)CLOCKS_PER_SEC / (float)count);
}

static void update_light_destruction(void)
{
  size_t count = 0;
  clock_t begin = clock();

  struct light_destructions curr_light_destructions = {0};
  while(light_destructions.item_count != 0)
  {
    SWAP(curr_light_destructions, light_destructions);
    for(size_t i=0; i<curr_light_destructions.item_count; ++i)
    {
      const struct light_destruction light_destruction = curr_light_destructions.items[i];

      // Esentially, we want to do is the following:
      //   1. Loop through each adjacent block
      //   2. Check we may have propagated light to each of them i.e. it is both
      //     1. Not opaque
      //     2. Have a lower light level lower than or equal to what we could propagate
      //     3: Non-zero ***
      //   3. Propagate if that is the case
      //   4. Otherwise, we would need to redo light propagation from them ***
      for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
      {
        struct cursor neighbour_cursor = light_destruction_cursor(light_destruction);
        if(cursor_move(&neighbour_cursor, direction))
        {
          const struct block_info *neighbour_block_info = query_block_info(cursor_get_block_id(neighbour_cursor));
          if(neighbour_block_info->type != BLOCK_TYPE_OPAQUE)
          {
            const unsigned light_level = light_destruction.light_level;
            const unsigned neighbour_light_level = cursor_get_block_light_level(neighbour_cursor);

            bool propagate = false;

            if(direction == DIRECTION_BOTTOM && light_level == 15 && neighbour_light_level == 15)
              propagate = true;
            else if(light_level == neighbour_light_level + 1)
              propagate = true;

            if(propagate)
            {
              cursor_set_block_light_level(neighbour_cursor, 0);
              enqueue_light_destroy_update(neighbour_cursor.chunk, neighbour_cursor.x, neighbour_cursor.y, neighbour_cursor.z, neighbour_light_level);
            }
            else if(neighbour_light_level != 0)
              enqueue_light_create_update(neighbour_cursor.chunk, neighbour_cursor.x, neighbour_cursor.y, neighbour_cursor.z);
          }
        }
      }
    }
    count += curr_light_destructions.item_count;
    curr_light_destructions.item_count = 0;
  }
  DYNAMIC_ARRAY_CLEAR(light_destructions);
  DYNAMIC_ARRAY_CLEAR(curr_light_destructions);

  clock_t end = clock();
  if(count != 0)
    LOG_INFO("Light System: Processed %zu destruction updates in %fs - Average %fs", count, (float)(end - begin) / (float)CLOCKS_PER_SEC, (float)(end - begin) / (float)CLOCKS_PER_SEC / (float)count);
}

void update_light(void)
{
  // Process light creation and destruction updates. We have to process
  // destruction updates first because we may create light creation updates
  // while creating light destruction updates.
  update_light_destruction();
  update_light_creation();
}

