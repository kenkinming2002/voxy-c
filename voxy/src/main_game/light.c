#include <voxy/main_game/light.h>
#include <voxy/main_game/world.h>
#include <voxy/main_game/mod.h>

#include <voxy/types/chunk.h>
#include <voxy/types/chunk_data.h>

#include <stdio.h>
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

//////////////////////////////
/// 1: Loading Light Level ///
//////////////////////////////

static inline struct light_info light_info_load_default()
{
  return (struct light_info){
    .opaque      = true,
    .ether       = 0,
    .light_level = 0,
  };
}

static inline struct light_info light_info_load_from_block(const struct block *block)
{
  const struct block_info *block_info = mod_block_info_get(block->id);
  return (struct light_info){
    .opaque      = block_info->type == BLOCK_TYPE_OPAQUE,
    .ether       = block_info->ether,
    .light_level = block_info->light_level,
  };
}

static inline struct light_info light_info_load_from_block_neighbour(const struct block *block)
{
  const struct block_info *block_info = mod_block_info_get(block->id);
  return (struct light_info){
    .opaque      = block_info->type == BLOCK_TYPE_OPAQUE,
    .ether       = block->ether,
    .light_level = block->light_level,
  };
}

static inline struct light_info light_info_load(struct chunk *chunk, ivec3_t position)
{
  if(position.x == -1          && (position.y >= 0 && position.y < CHUNK_WIDTH) && (position.z >= 0 && position.z < CHUNK_WIDTH)) return chunk->left   ? light_info_load_from_block_neighbour(&chunk->left  ->chunk_data->blocks[position.z][position.y][CHUNK_WIDTH-1]) : light_info_load_default();
  if(position.x == CHUNK_WIDTH && (position.y >= 0 && position.y < CHUNK_WIDTH) && (position.z >= 0 && position.z < CHUNK_WIDTH)) return chunk->right  ? light_info_load_from_block_neighbour(&chunk->right ->chunk_data->blocks[position.z][position.y][0]            ) : light_info_load_default();
  if(position.y == -1          && (position.x >= 0 && position.x < CHUNK_WIDTH) && (position.z >= 0 && position.z < CHUNK_WIDTH)) return chunk->back   ? light_info_load_from_block_neighbour(&chunk->back  ->chunk_data->blocks[position.z][CHUNK_WIDTH-1][position.x]) : light_info_load_default();
  if(position.y == CHUNK_WIDTH && (position.x >= 0 && position.x < CHUNK_WIDTH) && (position.z >= 0 && position.z < CHUNK_WIDTH)) return chunk->front  ? light_info_load_from_block_neighbour(&chunk->front ->chunk_data->blocks[position.z][0]            [position.x]) : light_info_load_default();
  if(position.z == -1          && (position.x >= 0 && position.x < CHUNK_WIDTH) && (position.y >= 0 && position.y < CHUNK_WIDTH)) return chunk->bottom ? light_info_load_from_block_neighbour(&chunk->bottom->chunk_data->blocks[CHUNK_WIDTH-1][position.y][position.x]) : light_info_load_default();
  if(position.z == CHUNK_WIDTH && (position.x >= 0 && position.x < CHUNK_WIDTH) && (position.y >= 0 && position.y < CHUNK_WIDTH)) return chunk->top    ? light_info_load_from_block_neighbour(&chunk->top   ->chunk_data->blocks[0]            [position.y][position.x]) : light_info_load_default();

  if((position.x >= 0 && position.x < CHUNK_WIDTH) && (position.y >= 0 && position.y < CHUNK_WIDTH) && (position.z >= 0 && position.z < CHUNK_WIDTH))
    return light_info_load_from_block(&chunk->chunk_data->blocks[position.z][position.y][position.x]);

  return light_info_load_default();
}

static inline void light_infos_load(struct light_infos *light_infos, struct chunk *chunk)
{
  for(int z=0; z<CHUNK_WIDTH+2; ++z)
    for(int y=0; y<CHUNK_WIDTH+2; ++y)
      for(int x=0; x<CHUNK_WIDTH+2; ++x)
        light_infos->items[z][y][x] = light_info_load(chunk, ivec3(x-1, y-1, z-1));
}

/////////////////////////////
/// 2: Saving Light Level ///
/////////////////////////////

static inline void light_info_save_to_block(struct block *block, struct light_info light_info)
{
  block->light_level = light_info.light_level;
  block->ether       = light_info.ether;
}

static inline bool light_info_save_to_block_neighbour(struct block *block, struct light_info light_info)
{
  return block->light_level != light_info.light_level || block->ether != light_info.ether;
}

static inline void light_info_save(struct chunk *chunk, ivec3_t position, struct light_info light_info)
{
  if(position.x == -1          && (position.y >= 0 && position.y < CHUNK_WIDTH) && (position.z >= 0 && position.z < CHUNK_WIDTH)) if(chunk->left)   { chunk->left  ->light_dirty |= light_info_save_to_block_neighbour(&chunk->left  ->chunk_data->blocks[position.z][position.y][CHUNK_WIDTH-1], light_info); };
  if(position.x == CHUNK_WIDTH && (position.y >= 0 && position.y < CHUNK_WIDTH) && (position.z >= 0 && position.z < CHUNK_WIDTH)) if(chunk->right)  { chunk->right ->light_dirty |= light_info_save_to_block_neighbour(&chunk->right ->chunk_data->blocks[position.z][position.y][0]            , light_info); };
  if(position.y == -1          && (position.x >= 0 && position.x < CHUNK_WIDTH) && (position.z >= 0 && position.z < CHUNK_WIDTH)) if(chunk->back)   { chunk->back  ->light_dirty |= light_info_save_to_block_neighbour(&chunk->back  ->chunk_data->blocks[position.z][CHUNK_WIDTH-1][position.x], light_info); };
  if(position.y == CHUNK_WIDTH && (position.x >= 0 && position.x < CHUNK_WIDTH) && (position.z >= 0 && position.z < CHUNK_WIDTH)) if(chunk->front)  { chunk->front ->light_dirty |= light_info_save_to_block_neighbour(&chunk->front ->chunk_data->blocks[position.z][0]            [position.x], light_info); };
  if(position.z == -1          && (position.x >= 0 && position.x < CHUNK_WIDTH) && (position.y >= 0 && position.y < CHUNK_WIDTH)) if(chunk->bottom) { chunk->bottom->light_dirty |= light_info_save_to_block_neighbour(&chunk->bottom->chunk_data->blocks[CHUNK_WIDTH-1][position.y][position.x], light_info); };
  if(position.z == CHUNK_WIDTH && (position.x >= 0 && position.x < CHUNK_WIDTH) && (position.y >= 0 && position.y < CHUNK_WIDTH)) if(chunk->top)    { chunk->top   ->light_dirty |= light_info_save_to_block_neighbour(&chunk->top   ->chunk_data->blocks[0]            [position.y][position.x], light_info); };

  if((position.x >= 0 && position.x < CHUNK_WIDTH) && (position.y >= 0 && position.y < CHUNK_WIDTH) && (position.z >= 0 && position.z < CHUNK_WIDTH))
    light_info_save_to_block(&chunk->chunk_data->blocks[position.z][position.y][position.x], light_info);
}

static inline void light_infos_save(struct light_infos *light_infos, struct chunk *chunk)
{
  for(int z=0; z<CHUNK_WIDTH+2; ++z)
    for(int y=0; y<CHUNK_WIDTH+2; ++y)
      for(int x=0; x<CHUNK_WIDTH+2; ++x)
        light_info_save(chunk, ivec3(x-1, y-1, z-1), light_infos->items[z][y][x]);
}

/////////////////////////
/// 3: Ether Handling ///
/////////////////////////

static inline void light_infos_propagate_ether(struct light_infos *light_infos)
{
  for(int z=CHUNK_WIDTH+1; z>0; --z)
    for(int y=1; y<CHUNK_WIDTH+1; ++y)
      for(int x=1; x<CHUNK_WIDTH+1; ++x)
      {
        struct light_info *light_info        = &light_infos->items[z]  [y][x];
        struct light_info *light_info_bottom = &light_infos->items[z-1][y][x];
        if(light_info->ether && !light_info_bottom->opaque)
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

//////////////////////
/// 4: Propagation ///
//////////////////////

struct light_update
{
  uint16_t x : 5;
  uint16_t y : 5;
  uint16_t z : 5;
};

struct light_update_layer
{
  struct light_update items[CHUNK_WIDTH*CHUNK_WIDTH*CHUNK_WIDTH+6*CHUNK_WIDTH*CHUNK_WIDTH];
  size_t              count;
};

struct light_update_stack
{
  struct light_update_layer layers[15];
};

static inline void light_update_stack_enqueue(struct light_update_stack *light_update_stack, int x, int y, int z, unsigned light_level)
{
  struct light_update_layer *light_update_layer = &light_update_stack->layers[light_level-1];
  struct light_update       *light_update       = &light_update_layer->items[light_update_layer->count++];

  light_update->x = x;
  light_update->y = y;
  light_update->z = z;
}

static inline void light_update_stack_propagate_set(struct light_update_stack *light_update_stack, int x, int y, int z, unsigned light_level, struct light_infos *light_infos)
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

  for(unsigned light_level = 15; light_level > 0; --light_level)
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

//////////////
/// 5: ??? ///
//////////////
void update_light(void)
{
  clock_t begin = clock();

  size_t count = 0;
  for(;;)
  {
    bool updated = false;
    world_chunk_for_each(chunk)
    {
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
        count += 1;
      }
    }
    if(!updated)
      break;
  }

  clock_t end = clock();
  if(count != 0)
    fprintf(stderr, "DEBUG: Light System: Processed %zu chunks in %fs - Average %fs\n", count, (float)(end - begin) / (float)CLOCKS_PER_SEC, (float)(end - begin) / (float)CLOCKS_PER_SEC / (float)count);
}
