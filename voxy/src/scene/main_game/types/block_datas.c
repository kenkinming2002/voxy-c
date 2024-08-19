#include <voxy/scene/main_game/types/block_datas.h>

void block_datas_init(struct block_datas *block_datas)
{
  DYNAMIC_ARRAY_INIT(*block_datas);
}

void block_datas_fini(struct block_datas *block_datas)
{
  DYNAMIC_ARRAY_CLEAR(*block_datas);
}

void block_datas_append(struct block_datas *block_datas, struct block_data block_data)
{
  DYNAMIC_ARRAY_APPEND(*block_datas, block_data);
}

