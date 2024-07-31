#ifndef BLOCK_H
#define BLOCK_H

#include <voxy/math/vector.h>

struct entity;
struct chunk;
struct block;

void block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position);
void block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position);

#endif // BLOCK_H
