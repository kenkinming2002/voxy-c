#ifndef VOXY_SCENE_MAIN_GAME_TYPES_BLOCK_DATAS_H
#define VOXY_SCENE_MAIN_GAME_TYPES_BLOCK_DATAS_H

#include <voxy/scene/main_game/types/block_data.h>
#include <libcommon/utils/dynamic_array.h>

DYNAMIC_ARRAY_DEFINE(block_datas, struct block_data);

void block_datas_init(struct block_datas *block_datas);
void block_datas_fini(struct block_datas *block_datas);
void block_datas_append(struct block_datas *block_datas, struct block_data block_data);

#endif // VOXY_SCENE_MAIN_GAME_TYPES_BLOCK_DATAS_H
