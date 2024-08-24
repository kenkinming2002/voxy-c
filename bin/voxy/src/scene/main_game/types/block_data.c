#include <voxy/scene/main_game/types/block_data.h>

ivec3_t block_data_position(const struct block_data *block_data)
{
  return ivec3(block_data->x, block_data->y, block_data->z);
}
