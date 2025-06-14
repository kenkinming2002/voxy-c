#ifndef LIGHT_LIGHT_H
#define LIGHT_LIGHT_H

#include <voxy/types.h>
#include <libmath/vector.h>
#include <stdint.h>

struct voxy_block_group;

void enqueue_light_destruction_update_at(struct voxy_block_group *block_group, ivec3_t position, voxy_light_t light);
void enqueue_light_creation_update_at(struct voxy_block_group *block_group, ivec3_t position);

void enqueue_light_destruction_update(ivec3_t position, voxy_light_t light);
void enqueue_light_creation_update(ivec3_t position);

void light_update(void);

#endif // LIGHT_LIGHT_H
