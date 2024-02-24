#ifndef VOXY_MAIN_GAME_CHUNK_H
#define VOXY_MAIN_GAME_CHUNK_H

#include <voxy/main_game/config.h>
#include <voxy/main_game/block.h>
#include <voxy/math/vector.h>

#include <glad/glad.h>

#include <stddef.h>
#include <stdbool.h>

struct chunk
{
  /*******
   * Key *
   *******/
  struct
  {
    ivec3_t position;
    size_t  hash;
  };

  /*********
   * Links *
   *********/
  struct
  {
    struct chunk *next;
    struct chunk *bottom;
    struct chunk *top;
    struct chunk *back;
    struct chunk *front;
    struct chunk *left;
    struct chunk *right;
  };

  /*****************
   * Invalidations *
   *****************/
  struct
  {
    bool          mesh_invalidated;
    struct chunk *mesh_next;

    bool          light_invalidated;
    struct chunk *light_next;
  };

  /********
   * Data *
   ********/
  struct
  {
    struct block blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];

    struct entity **entities;
    size_t          entity_count;
    size_t          entity_capacity;
  };

  /*************
   * Rendering *
   *************/
  struct
  {
    bool culled;

    GLuint vao_opaque;
    GLuint vbo_opaque;
    GLsizei count_opaque;

    GLuint vao_transparent;
    GLuint vbo_transparent;
    GLsizei count_transparent;
  };
};

/*
 * Get pointer to block near a chunk, where position is in chunk local
 * coordinate.
 *
 * When position.x, position.y, position.z are in the range [0, CHUNK_WIDTH),
 * the returned block lie within supplied chunk. Otherwise, neighbour chunks
 * will be traversed recursively. If chunk is NULL, or chunk becomes NULL during
 * traversal, NULL is returned. This function is intended for used only when the
 * queried block is close.
 */
struct block *chunk_get_block(struct chunk *chunk, ivec3_t position);

/*
 * Add an entity to a chunk.
 */
void chunk_add_entity(struct chunk *chunk, struct entity *entity);

#endif // VOXY_MAIN_GAME_CHUNK_H
