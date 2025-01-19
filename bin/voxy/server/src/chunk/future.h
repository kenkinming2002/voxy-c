#ifndef CHUNK_FUTURE_H
#define CHUNK_FUTURE_H

#include <stdbool.h>

struct chunk_future
{
  struct voxy_chunk *value;
  bool pending;
};

#define chunk_future_ready(chunk) (struct chunk_future) { .value = chunk, }
#define chunk_future_pending      (struct chunk_future) { .value = NULL, .pending = true, }
#define chunk_future_reject       (struct chunk_future) { .value = NULL, .pending = false, }

#endif // CHUNK_FUTURE_H
