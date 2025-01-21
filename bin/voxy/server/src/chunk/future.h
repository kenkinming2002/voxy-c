#ifndef CHUNK_FUTURE_H
#define CHUNK_FUTURE_H

#include <stdbool.h>

struct chunk_future
{
  struct voxy_chunk *value;
  bool pending;
};

#define chunk_future_ready(v) (struct chunk_future) { .pending = false, .value = v, }
#define chunk_future_pending  (struct chunk_future) { .pending = true, }

struct unit_future
{
  bool pending;
};

#define unit_future_ready    (struct unit_future) { .pending = false, }
#define unit_future_pending  (struct unit_future) { .pending = true, }

#endif // CHUNK_FUTURE_H
