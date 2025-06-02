#include "manager.h"

#include <stb_ds.h>
#include <empty.h>

struct active_chunk *active_chunks;

void voxy_reset_active_chunks(void)
{
  hmfree(active_chunks);
}

void voxy_add_active_chunk(ivec3_t position)
{
  hmput(active_chunks, position, (struct empty){});
}

