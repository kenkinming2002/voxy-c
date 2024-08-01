#include <voxy/scene/main_game/update/chunk_manager.h>

#include <voxy/scene/main_game/update/light.h>
#include <voxy/scene/main_game/update/chunk_database.h>
#include <voxy/scene/main_game/update/chunk_generate.h>

#include <voxy/scene/main_game/states/chunks.h>

#include <voxy/scene/main_game/types/chunk.h>
#include <voxy/scene/main_game/types/chunk_data.h>

#include <voxy/core/log.h>
#include <voxy/core/fs.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct ivec3_node
{
  struct ivec3_node *next;
  size_t hash;
  ivec3_t value;
};

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX ivec3
#define SC_HASH_TABLE_NODE_TYPE struct ivec3_node
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>

ivec3_t ivec3_key(struct ivec3_node *node)
{
  return node->value;
}

int ivec3_compare(ivec3_t position1, ivec3_t position2)
{
  if(position1.x != position2.x) return position1.x - position2.x;
  if(position1.y != position2.y) return position1.y - position2.y;
  if(position1.z != position2.z) return position1.z - position2.z;
  return 0;
}

void ivec3_dispose(struct ivec3_node *node)
{
  free(node);
}

static struct ivec3_hash_table active_chunk_positions;

void reset_active_chunks(void)
{
  ivec3_hash_table_dispose(&active_chunk_positions);
}

void activate_chunk(ivec3_t position)
{
  struct ivec3_node *node = malloc(sizeof *node);
  node->value = position;
  ivec3_hash_table_insert(&active_chunk_positions, node);
}

void sync_active_chunks(void)
{
  int load_count = 0;
  int save_count = 0;

  int generate_count = 0;
  int discard_count = 0;

  // Save/Discard chunk
  world_for_each_chunk(chunk)
  {
    if(chunk->data && chunk_is_dirty(chunk) && save_chunk_data(chunk->position, chunk->data))
    {
      chunk->data->dirty = false;
      save_count += 1;
    }

    if(chunk->data && !chunk_is_dirty(chunk) && !ivec3_hash_table_lookup(&active_chunk_positions, chunk->position))
    {
      chunk_data_destroy(chunk->data);
      chunk->data = NULL;
      discard_count += 1;
    }
  }

  // Load/Generate chunks
  for(size_t i=0; i<active_chunk_positions.bucket_count; ++i)
    for(struct ivec3_node *node = active_chunk_positions.buckets[i].head; node; node = node->next)
    {
      const ivec3_t chunk_position = node->value;
      struct chunk *chunk = world_get_chunk(chunk_position);

      if(!chunk->data && !chunk->busy)
      {
        load_count += 1;
        chunk->data = load_chunk_data(chunk->position);
        if(chunk->data)
        {
          chunk->mesh_invalidated = true;
          if(chunk->left) chunk->left->mesh_invalidated = true;
          if(chunk->right) chunk->right->mesh_invalidated = true;
          if(chunk->back) chunk->back->mesh_invalidated = true;
          if(chunk->front) chunk->front->mesh_invalidated = true;
          if(chunk->bottom) chunk->bottom->mesh_invalidated = true;
          if(chunk->top) chunk->top->mesh_invalidated = true;
        }
      }

      if(!chunk->data && !chunk->busy && enqueue_generate_chunk(chunk))
        chunk->busy = true;

      if(chunk->busy && (chunk->data = atomic_exchange_explicit(&chunk->new_data, NULL, memory_order_acquire)))
      {
        chunk->busy = false;

        chunk->mesh_invalidated = true;
        if(chunk->left) chunk->left->mesh_invalidated = true;
        if(chunk->right) chunk->right->mesh_invalidated = true;
        if(chunk->back) chunk->back->mesh_invalidated = true;
        if(chunk->front) chunk->front->mesh_invalidated = true;
        if(chunk->bottom) chunk->bottom->mesh_invalidated = true;
        if(chunk->top) chunk->top->mesh_invalidated = true;

        for(int z = 0; z<CHUNK_WIDTH; ++z)
          for(int y = 0; y<CHUNK_WIDTH; ++y)
            for(int x = 0; x<CHUNK_WIDTH; ++x)
            {
              const ivec3_t position = ivec3(x, y, z);
              if(chunk_get_block_light_level(chunk, position) != 0)
                enqueue_light_create_update(chunk, x, y, z);
              else if(z == 0 || z == CHUNK_WIDTH - 1 || y == 0 || y == CHUNK_WIDTH - 1 || x == 0 || x == CHUNK_WIDTH - 1)
                enqueue_light_destroy_update(chunk, x, y, z, 0);
            }

        generate_count += 1;
      }

    }

  if(generate_count != 0)
    LOG_INFO("Chunk Manager: Generarted %d chunks in background", generate_count);

  if(discard_count != 0)
    LOG_INFO("Chunk Manager: Discarded %d chunks", discard_count);

  if(load_count != 0)
    LOG_INFO("Chunk Manager: Loaded %d chunks", load_count);

  if(save_count != 0)
    LOG_INFO("Chunk Manager: Saved %d chunks", save_count);
}

void save_active_chunks(void)
{
  char dirpath[] = WORLD_DIRPATH "/chunks";
  const char *filepath = WORLD_DIRPATH "/chunks/active";

  if(mkdir_recursive(dirpath) != 0)
  {
    LOG_ERROR("Failed to save active chunks: Failed to create directory: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  FILE *f = fopen(filepath, "wb");
  if(!f)
  {
    LOG_ERROR("Failed to save active chunks: Failed to open file");
    exit(EXIT_FAILURE);
  }

  for(size_t i=0; i<active_chunk_positions.bucket_count; ++i)
    for(struct ivec3_node *node = active_chunk_positions.buckets[i].head; node; node = node->next)
    {
      const ivec3_t active_chunk_position = node->value;
      if(fwrite(&active_chunk_position, sizeof active_chunk_position, 1, f) != 1)
      {
        LOG_ERROR("Failed to save active chunks: Failed to write to file");
        exit(EXIT_FAILURE);
      }
    }
}

void load_active_chunks(void)
{
  const char *filepath = WORLD_DIRPATH "/chunks/active";

  FILE *f = fopen(filepath, "rb");
  if(!f)
    return;

  ivec3_t active_chunk_position;
  while(fread(&active_chunk_position, sizeof active_chunk_position, 1, f) == 1)
  {
    struct ivec3_node *node = malloc(sizeof *node);
    node->value = active_chunk_position;
    ivec3_hash_table_insert(&active_chunk_positions, node);
  }

  if(!feof(f))
    LOG_WARN("Error when loading active chunks: File maybe corrupted!?");

  fclose(f);
}

