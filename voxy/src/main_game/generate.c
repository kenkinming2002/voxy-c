#include <voxy/main_game/generate.h>

#include <assert.h>

static generate_chunk_blocks_t generate_chunk_blocks_func;
static generate_player_spawn_t generate_player_spawn_func;

void register_generate_chunk_blocks(generate_chunk_blocks_t func)
{
  assert(!generate_chunk_blocks_func);
  generate_chunk_blocks_func = func;
}

void register_generate_player_spawn(generate_player_spawn_t func)
{
  assert(!generate_player_spawn_func);
  generate_player_spawn_func = func;
}

void generate_chunk_blocks(seed_t seed, ivec3_t position, block_id_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH])
{
  assert(generate_chunk_blocks_func);
  generate_chunk_blocks_func(seed, position, blocks);
}

fvec3_t generate_player_spawn(seed_t seed)
{
  assert(generate_player_spawn_func);
  return generate_player_spawn_func(seed);
}
