#ifndef VOXY_WORLD_UPDATE_CHUNK_GENERATE_H
#define VOXY_WORLD_UPDATE_CHUNK_GENERATE_H

struct world;
struct resource_pack;
struct world_generator;

void world_update_chunk_generate(struct world *world, struct resource_pack *resource_pack, struct world_generator *world_generator);

#endif // VOXY_WORLD_UPDATE_CHUNK_GENERATE_H
