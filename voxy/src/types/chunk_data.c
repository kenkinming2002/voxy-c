#include <types/chunk_data.h>
#include <stdlib.h>

void chunk_data_dispose(struct chunk_data *chunk_data)
{
  free(chunk_data);
}
