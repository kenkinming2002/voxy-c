#ifndef BLOCK_H
#define BLOCK_H

#include <voxy/math/vector.h>

struct entity;
struct chunk;
struct block;

void block_on_create(struct entity *entity, struct chunk *chunk, struct block *block);
void block_on_destroy(struct entity *entity, struct chunk *chunk, struct block *block);

#endif // BLOCK_H
