#ifndef VOXY_TYPES_H
#define VOXY_TYPES_H

#include <stdint.h>

typedef uint8_t voxy_block_id_t;
typedef uint8_t voxy_entity_id_t;
typedef uint8_t voxy_item_id_t;

typedef struct
{
  uint8_t level : 4;
  uint8_t sol : 4; // We use up all the remaining bits to avoid any funny business
}
voxy_light_t;

#endif // VOXY_TYPES_H
