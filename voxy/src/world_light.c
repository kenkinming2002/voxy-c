#include "world.h"

#include <string.h>
#include <stdio.h>

/// Rule for light update
///
/// 0: Block
///  - Opaque block always has light level of 0 regardless of following rules.
///
/// 1: Ether
///  - Tile with z above or equal ETHER_HEIGHT always has ether set.
///  - If tile above has ether set, current tile also has ether set.
///  - Ether tile has light level 15 during day
///
/// 2: Neighbour
///  - If tile does not have ether set or for light level during the night,
///    tile light level is minimum of light levels of neighbour tiles
///    subtract 1.
///  - If the computed light level is below 0, it will be set to 0.
///

static inline bool    tile_is_opaque  (struct tile tile) { return tile.id != TILE_ID_EMPTY && tile.id != TILE_ID_ETHER; }
static inline bool    tile_is_ether   (struct tile tile) { return tile.id == TILE_ID_ETHER; }
static inline uint8_t tile_light_level(struct tile tile) { (void)tile; return 0; }

struct light_info
{
  uint8_t opaque      : 1;
  uint8_t ether       : 1;
  uint8_t light_level : 4;
};

struct light_infos
{
  struct light_info items[CHUNK_WIDTH+2][CHUNK_WIDTH+2][CHUNK_WIDTH+2];
};

static inline struct tile       *get_tile      (struct chunk       *chunk,       int x, int y, int z) { return &chunk->tiles[z][y][x]; }
static inline struct light_info *get_light_info(struct light_infos *light_infos, int x, int y, int z) { return &light_infos->items[z+1][y+1][x+1]; }

static inline void light_infos_load(struct light_infos *light_infos, struct chunk *chunk)
{
  for(int z=0; z<CHUNK_WIDTH+2; ++z)
    for(int y=0; y<CHUNK_WIDTH+2; ++y)
      for(int x=0; x<CHUNK_WIDTH+2; ++x)
      {
        light_infos->items[z][y][x].opaque      = true;
        light_infos->items[z][y][x].ether       = false;
        light_infos->items[z][y][x].light_level = 0;
      }

  for(int z=0; z<CHUNK_WIDTH; ++z)
    for(int y=0; y<CHUNK_WIDTH; ++y)
      for(int x=0; x<CHUNK_WIDTH; ++x)
      {
        struct tile       *tile       = get_tile      (chunk,       x, y, z);
        struct light_info *light_info = get_light_info(light_infos, x, y, z);

        light_info->opaque      = tile_is_opaque(*tile);
        light_info->ether       = tile_is_ether(*tile);
        light_info->light_level = tile_light_level(*tile);
      }

  if(chunk->bottom)
    for(int y=0; y<CHUNK_WIDTH; ++y)
      for(int x=0; x<CHUNK_WIDTH; ++x)
      {
        struct tile       *tile       = get_tile      (chunk->bottom, x, y, CHUNK_WIDTH-1);
        struct light_info *light_info = get_light_info(light_infos  , x, y, -1           );

        light_info->opaque      = tile_is_opaque(*tile);
        light_info->ether       = tile->ether;
        light_info->light_level = tile->light_level;
      }

  if(chunk->top)
    for(int y=0; y<CHUNK_WIDTH; ++y)
      for(int x=0; x<CHUNK_WIDTH; ++x)
      {
        struct tile       *tile       = get_tile      (chunk->top , x, y, 0          );
        struct light_info *light_info = get_light_info(light_infos, x, y, CHUNK_WIDTH);

        light_info->opaque      = tile_is_opaque(*tile);
        light_info->ether       = tile->ether;
        light_info->light_level = tile->light_level;
      }

  if(chunk->back)
    for(int z=0; z<CHUNK_WIDTH; ++z)
      for(int x=0; x<CHUNK_WIDTH; ++x)
      {
        struct tile       *tile       = get_tile      (chunk->back, x, CHUNK_WIDTH-1, z);
        struct light_info *light_info = get_light_info(light_infos, x, -1           , z);

        light_info->opaque      = tile_is_opaque(*tile);
        light_info->ether       = tile->ether;
        light_info->light_level = tile->light_level;
      }

  if(chunk->front)
    for(int z=0; z<CHUNK_WIDTH; ++z)
      for(int x=0; x<CHUNK_WIDTH; ++x)
      {
        struct tile       *tile       = get_tile      (chunk->front, x, 0          , z);
        struct light_info *light_info = get_light_info(light_infos , x, CHUNK_WIDTH, z);

        light_info->opaque      = tile_is_opaque(*tile);
        light_info->ether       = tile->ether;
        light_info->light_level = tile->light_level;
      }

  if(chunk->left)
    for(int z=0; z<CHUNK_WIDTH; ++z)
      for(int y=0; y<CHUNK_WIDTH; ++y)
      {
        struct tile       *tile       = get_tile      (chunk->left, CHUNK_WIDTH-1, y, z);
        struct light_info *light_info = get_light_info(light_infos, -1           , y, z);

        light_info->opaque      = tile_is_opaque(*tile);
        light_info->ether       = tile->ether;
        light_info->light_level = tile->light_level;
      }

  if(chunk->right)
    for(int z=0; z<CHUNK_WIDTH; ++z)
      for(int y=0; y<CHUNK_WIDTH; ++y)
      {
        struct tile       *tile       = get_tile      (chunk->right, 0          , y, z);
        struct light_info *light_info = get_light_info(light_infos , CHUNK_WIDTH, y, z);

        light_info->opaque      = tile_is_opaque(*tile);
        light_info->ether       = tile->ether;
        light_info->light_level = tile->light_level;
      }
}

static inline void light_infos_save(struct light_infos *light_infos, struct chunk *chunk)
{
  for(int z=0; z<CHUNK_WIDTH; ++z)
    for(int y=0; y<CHUNK_WIDTH; ++y)
      for(int x=0; x<CHUNK_WIDTH; ++x)
      {
        struct tile       *tile       = get_tile      (chunk,       x, y, z);
        struct light_info *light_info = get_light_info(light_infos, x, y, z);

        tile->ether       = light_info->ether;
        tile->light_level = light_info->light_level;
      }

  if(chunk->bottom && !chunk->bottom->light_dirty)
  {
    __label__ done;
    for(int y=0; y<CHUNK_WIDTH; ++y)
      for(int x=0; x<CHUNK_WIDTH; ++x)
      {
        struct tile       *tile       = get_tile      (chunk->bottom, x, y, CHUNK_WIDTH-1);
        struct light_info *light_info = get_light_info(light_infos  , x, y, -1           );
        if(tile->ether != light_info->ether || tile->light_level != light_info->light_level)
        {
          chunk->bottom->light_dirty = true;
          goto done;
        }
      }
    done:;
  }

  if(chunk->top && !chunk->top->light_dirty)
  {
    __label__ done;
    for(int y=0; y<CHUNK_WIDTH; ++y)
      for(int x=0; x<CHUNK_WIDTH; ++x)
      {
        struct tile       *tile       = get_tile      (chunk->top , x, y, 0          );
        struct light_info *light_info = get_light_info(light_infos, x, y, CHUNK_WIDTH);
        if(tile->ether != light_info->ether || tile->light_level != light_info->light_level)
        {
          chunk->top->light_dirty = true;
          goto done;
        }
      }
    done:;
  }

  if(chunk->back && !chunk->back->light_dirty)
  {
    __label__ done;
    for(int z=0; z<CHUNK_WIDTH; ++z)
      for(int x=0; x<CHUNK_WIDTH; ++x)
      {
        struct tile       *tile       = get_tile      (chunk->back, x, CHUNK_WIDTH-1, z);
        struct light_info *light_info = get_light_info(light_infos, x, -1           , z);
        if(tile->ether != light_info->ether || tile->light_level != light_info->light_level)
        {
          chunk->back->light_dirty = true;
          goto done;
        }
      }
    done:;
  }

  if(chunk->front && !chunk->front->light_dirty)
  {
    __label__ done;
    for(int z=0; z<CHUNK_WIDTH; ++z)
      for(int x=0; x<CHUNK_WIDTH; ++x)
      {
        struct tile       *tile       = get_tile      (chunk->front, x, 0          , z);
        struct light_info *light_info = get_light_info(light_infos , x, CHUNK_WIDTH, z);
        if(tile->ether != light_info->ether || tile->light_level != light_info->light_level)
        {
          chunk->front->light_dirty = true;
          goto done;
        }
      }
    done:;
  }

  if(chunk->left && !chunk->left->light_dirty)
  {
    __label__ done;
    for(int z=0; z<CHUNK_WIDTH; ++z)
      for(int y=0; y<CHUNK_WIDTH; ++y)
      {
        struct tile       *tile       = get_tile      (chunk->left, CHUNK_WIDTH-1, y, z);
        struct light_info *light_info = get_light_info(light_infos, -1           , y, z);
        if(tile->ether != light_info->ether || tile->light_level != light_info->light_level)
        {
          chunk->left->light_dirty = true;
          goto done;
        }
      }
    done:;
  }

  if(chunk->right && !chunk->right->light_dirty)
  {
    __label__ done;
    for(int z=0; z<CHUNK_WIDTH; ++z)
      for(int y=0; y<CHUNK_WIDTH; ++y)
      {
        struct tile       *tile       = get_tile      (chunk->right, 0          , y, z);
        struct light_info *light_info = get_light_info(light_infos , CHUNK_WIDTH, y, z);
        if(tile->ether != light_info->ether || tile->light_level != light_info->light_level)
        {
          chunk->right->light_dirty = true;
          goto done;
        }
      }
    done:;
  }
}

static inline void light_infos_propagate_ether(struct light_infos *light_infos)
{
  for(int z=CHUNK_WIDTH; z>=0; --z)
    for(int y=0; y<CHUNK_WIDTH; ++y)
      for(int x=0; x<CHUNK_WIDTH; ++x)
      {
        struct light_info *light_info        = get_light_info(light_infos, x, y, z);
        struct light_info *light_info_bottom = get_light_info(light_infos, x, y, z-1);
        if(light_info->ether)
          if(!light_info_bottom->opaque)
            light_info_bottom->ether = true;
      }
}

static inline void light_infos_apply_ether(struct light_infos *light_infos)
{
  for(int z=0; z<CHUNK_WIDTH+2; ++z)
    for(int y=0; y<CHUNK_WIDTH+2; ++y)
      for(int x=0; x<CHUNK_WIDTH+2; ++x)
        if(light_infos->items[z][y][x].ether)
          light_infos->items[z][y][x].light_level = 15;
}

struct light_update
{
  int8_t x;
  int8_t y;
  int8_t z;
};

struct light_update_layer
{
  struct light_update items[(CHUNK_WIDTH+2) * (CHUNK_WIDTH+2) * (CHUNK_WIDTH+2)];
  size_t              count;
};

struct light_update_stack
{
  struct light_update_layer layers[15];
};

static inline void light_update_stack_enqueue(struct light_update_stack *light_update_stack, int x, int y, int z, uint8_t light_level)
{
  struct light_update_layer *light_update_layer = &light_update_stack->layers[light_level-1];
  struct light_update       *light_update       = &light_update_layer->items[light_update_layer->count++];

  light_update->x = x;
  light_update->y = y;
  light_update->z = z;
}

static inline void light_update_stack_propagate_set(struct light_update_stack *light_update_stack, int x, int y, int z, uint8_t light_level, struct light_infos *light_infos)
{
  if(x < 0 || x >= CHUNK_WIDTH+2) return;
  if(y < 0 || y >= CHUNK_WIDTH+2) return;
  if(z < 0 || z >= CHUNK_WIDTH+2) return;

  struct light_info *light_info = &light_infos->items[z][y][x];
  if(!light_info->opaque && light_info->light_level < light_level)
  {
    light_info->light_level = light_level;
    light_update_stack_enqueue(light_update_stack, x, y, z, light_level);
  }
}

static inline void light_infos_propagate_light(struct light_infos *light_infos)
{
  struct light_update_stack light_update_stack;
  for(unsigned i=0; i<15; ++i)
    light_update_stack.layers[i].count = 0;

  for(int z=0; z<CHUNK_WIDTH+2; ++z)
    for(int y=0; y<CHUNK_WIDTH+2; ++y)
      for(int x=0; x<CHUNK_WIDTH+2; ++x)
        if(light_infos->items[z][y][x].light_level != 0)
          light_update_stack_enqueue(&light_update_stack, x, y, z, light_infos->items[z][y][x].light_level);

  for(uint8_t light_level = 15; light_level > 0; --light_level)
  {
    struct light_update_layer *light_update_layer = &light_update_stack.layers[light_level-1];
    for(size_t i=0; i<light_update_layer->count; ++i)
    {
      struct light_update *light_update = &light_update_layer->items[i];
      struct light_info   *light_info   = &light_infos->items[light_update->z][light_update->y][light_update->x];
      if(light_info->light_level == light_level)
      {
        light_update_stack_propagate_set(&light_update_stack, light_update->x-1, light_update->y, light_update->z, light_level-1, light_infos);
        light_update_stack_propagate_set(&light_update_stack, light_update->x+1, light_update->y, light_update->z, light_level-1, light_infos);
        light_update_stack_propagate_set(&light_update_stack, light_update->x, light_update->y-1, light_update->z, light_level-1, light_infos);
        light_update_stack_propagate_set(&light_update_stack, light_update->x, light_update->y+1, light_update->z, light_level-1, light_infos);
        light_update_stack_propagate_set(&light_update_stack, light_update->x, light_update->y, light_update->z-1, light_level-1, light_infos);
        light_update_stack_propagate_set(&light_update_stack, light_update->x, light_update->y, light_update->z+1, light_level-1, light_infos);
      }
    }
  }
}

void world_update_light(struct world *world)
{
  size_t count = 0;

  for(;;)
  {
    bool updated = false;
    for(size_t i=0; i<world->chunks.bucket_count; ++i)
      for(struct chunk *chunk = world->chunks.buckets[i].head; chunk; chunk = chunk->next)
        if(chunk->light_dirty)
        {
          struct light_infos light_infos;

          light_infos_load(&light_infos, chunk);
          light_infos_propagate_ether(&light_infos);
          light_infos_apply_ether(&light_infos);
          light_infos_propagate_light(&light_infos);
          light_infos_save(&light_infos, chunk);

          chunk->light_dirty = false;
          chunk->mesh_dirty = true;
          if(chunk->left)   chunk->left  ->mesh_dirty = true;
          if(chunk->right)  chunk->right ->mesh_dirty = true;
          if(chunk->back)   chunk->back  ->mesh_dirty = true;
          if(chunk->front)  chunk->front ->mesh_dirty = true;
          if(chunk->bottom) chunk->bottom->mesh_dirty = true;
          if(chunk->top)    chunk->top   ->mesh_dirty = true;

          updated = true;
          ++count;
        }

    if(!updated)
      break;
  }

  if(count != 0)
    fprintf(stderr, "DEBUG: Light System: Processed %zu chunks\n", count);
}
