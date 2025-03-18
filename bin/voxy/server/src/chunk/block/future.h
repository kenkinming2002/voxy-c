#ifndef CHUNK_BLOCK_FUTURE_H
#define CHUNK_BLOCK_FUTURE_H

#include <stdbool.h>

struct block_group_future
{
  struct voxy_block_group *value;
  bool pending;
};

#define block_group_future_ready(v) (struct block_group_future) { .pending = false, .value = v, }
#define block_group_future_pending  (struct block_group_future) { .pending = true, }

struct unit_future
{
  bool pending;
};

#define unit_future_ready    (struct unit_future) { .pending = false, }
#define unit_future_pending  (struct unit_future) { .pending = true, }

#endif // CHUNK_BLOCK_FUTURE_H
