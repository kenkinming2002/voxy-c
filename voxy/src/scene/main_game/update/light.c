#include <voxy/scene/main_game/update/light.h>

#include <voxy/scene/main_game/types/chunk.h>
#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/cursor.h>

#include <voxy/core/log.h>

#include <sys/queue.h>

#include <stdlib.h>
#include <time.h>

/// Rule for light update
///
/// 0: Block
///  - Opaque block always has light level of 0 regardless of following rules.
///
/// 1: Ether
///  - Block with z above or equal ETHER_HEIGHT always has ether set.
///  - If block above has ether set, current block also has ether set.
///  - Ether block has light level 15 during day
///
/// 2: Neighbour
///  - If block does not have ether set or for light level during the night,
///    block light level is minimum of light levels of neighbour blocks
///    subtract 1.
///  - If the computed light level is below 0, it will be set to 0.
///
/// The rules are simple. The implementation, not so much.

// We need to store light creation/destruction in a fifo data structure i.e.
// queue but obviously we do not have a implementation in C. We are using the
// singly linked list from BSD. This is both non-portable and inefficien
// considering the size of our structure is so small.
struct light_creation
{
  STAILQ_ENTRY(light_creation) link;
  struct cursor cursor;
};
STAILQ_HEAD(light_creations, light_creation);

// We need to store light creation/destruction in a fifo data structure i.e.
// queue but obviously we do not have a implementation in C. We are using the
// singly linked list from BSD. This is both non-portable and inefficien
// considering the size of our structure is so small.
struct light_destruction
{
  STAILQ_ENTRY(light_destruction) link;
  struct cursor cursor;
  uint8_t ether : 1;
  uint8_t light_level : 4;
};
STAILQ_HEAD(light_destructions, light_destruction);

struct light_creations light_creations = STAILQ_HEAD_INITIALIZER(light_creations);
struct light_destructions light_destructions = STAILQ_HEAD_INITIALIZER(light_destructions);

void enqueue_light_create_update(struct chunk *chunk, unsigned x, unsigned y, unsigned z)
{
  struct light_creation *p_light_creation = malloc(sizeof *p_light_creation);
  p_light_creation->cursor.chunk = chunk;
  p_light_creation->cursor.x = x;
  p_light_creation->cursor.y = y;
  p_light_creation->cursor.z = z;
  STAILQ_INSERT_TAIL(&light_creations, p_light_creation, link);
}

void enqueue_light_destroy_update(struct chunk *chunk, unsigned x, unsigned y, unsigned z, unsigned light_level, unsigned ether)
{
  struct light_destruction *p_light_destruction = malloc(sizeof *p_light_destruction);
  p_light_destruction->cursor.chunk = chunk;
  p_light_destruction->cursor.x = x;
  p_light_destruction->cursor.y = y;
  p_light_destruction->cursor.z = z;
  p_light_destruction->light_level = light_level;
  p_light_destruction->ether = ether;
  STAILQ_INSERT_TAIL(&light_destructions, p_light_destruction, link);
}

static void update_light_creation(void)
{
  size_t count = 0;
  clock_t begin = clock();

  struct light_creation *light_creation;
  while((light_creation = STAILQ_FIRST(&light_creations)))
  {
    STAILQ_REMOVE_HEAD(&light_creations, link);
    count += 1;

    // Esentially, we want to do is the following:
    //   1. Loop through each adjacent block
    //   2. Check we can propagate light to each of them i.e. it is both
    //     1. Not opaque
    //     2. Have a lower light level strictly lower than what we could propagate
    //   3. Propagate if that is the case

    struct cursor cursor = light_creation->cursor;
    for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
    {
      struct cursor neighbour_cursor = light_creation->cursor;
      if(cursor_move(&neighbour_cursor, direction))
      {
        const struct block_info *neighbour_block_info = query_block_info(cursor_get_block_id(neighbour_cursor));
        if((neighbour_block_info->type != BLOCK_TYPE_OPAQUE) && ((direction == DIRECTION_BOTTOM && (int)cursor_get_block_ether(neighbour_cursor) < (int)cursor_get_block_ether(cursor)) || (int)cursor_get_block_light_level(neighbour_cursor) < (int)cursor_get_block_light_level(cursor) - 1))
        {
          struct light_creation *new_light_creation = malloc(sizeof *new_light_creation);
          new_light_creation->cursor = neighbour_cursor;
          STAILQ_INSERT_TAIL(&light_creations, new_light_creation, link);

          cursor_set_block_ether(neighbour_cursor, direction == DIRECTION_BOTTOM ? cursor_get_block_ether(cursor) : 0);
          cursor_set_block_light_level(neighbour_cursor, cursor_get_block_ether(neighbour_cursor) ? 15 : (int)cursor_get_block_light_level(cursor) - 1);
        }
      }
    }

    free(light_creation);
  }

  clock_t end = clock();
  if(count != 0)
    LOG_INFO("Light System: Processed %zu creation updates in %fs - Average %fs", count, (float)(end - begin) / (float)CLOCKS_PER_SEC, (float)(end - begin) / (float)CLOCKS_PER_SEC / (float)count);
}

static void update_light_destruction(void)
{
  size_t count = 0;
  clock_t begin = clock();

  struct light_destruction *light_destruction;
  while((light_destruction = STAILQ_FIRST(&light_destructions)))
  {
    STAILQ_REMOVE_HEAD(&light_destructions, link);
    count += 1;

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
      struct cursor neighbour_cursor = light_destruction->cursor;
      if(cursor_move(&neighbour_cursor, direction))
      {
        const struct block_info *neighbour_block_info = query_block_info(cursor_get_block_id(neighbour_cursor));

        if(neighbour_block_info->type != BLOCK_TYPE_OPAQUE && ((direction == DIRECTION_BOTTOM && (int)cursor_get_block_ether(neighbour_cursor) != 0 && (int)cursor_get_block_ether(neighbour_cursor) <= (int)light_destruction->ether) || ((int)cursor_get_block_light_level(neighbour_cursor) != 0 && (int)cursor_get_block_light_level(neighbour_cursor) <= (int)light_destruction->light_level - 1)))
        {
          struct light_destruction *new_light_destruction = malloc(sizeof *new_light_destruction);
          new_light_destruction->cursor = neighbour_cursor;
          new_light_destruction->light_level = cursor_get_block_light_level(neighbour_cursor);
          new_light_destruction->ether = cursor_get_block_ether(neighbour_cursor);
          STAILQ_INSERT_TAIL(&light_destructions, new_light_destruction, link);

          cursor_set_block_ether(neighbour_cursor, 0);
          cursor_set_block_light_level(neighbour_cursor, 0);
        }
        else if(cursor_get_block_ether(neighbour_cursor) != 0 || cursor_get_block_light_level(neighbour_cursor) != 0)
        {
          struct light_creation *new_light_creation = malloc(sizeof *new_light_creation);
          new_light_creation->cursor = neighbour_cursor;
          STAILQ_INSERT_TAIL(&light_creations, new_light_creation, link);
        }
      }
    }
    free(light_destruction);
  }

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

