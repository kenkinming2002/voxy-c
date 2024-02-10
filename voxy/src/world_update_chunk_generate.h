#ifndef VOXY_WORLD_UPDATE_CHUNK_GENERATE_H
#define VOXY_WORLD_UPDATE_CHUNK_GENERATE_H

struct world;
struct mod;
struct world_generator;

void world_update_chunk_generate(struct world *world, struct mod *mod, struct world_generator *world_generator);

#endif // VOXY_WORLD_UPDATE_CHUNK_GENERATE_H
