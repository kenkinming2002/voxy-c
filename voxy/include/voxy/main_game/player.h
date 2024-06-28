#ifndef VOXY_MAIN_GAME_PLAYER_H
#define VOXY_MAIN_GAME_PLAYER_H

#include <voxy/main_game/entity.h>

#include <stdbool.h>

void player_entity_init(struct entity *entity);
void player_entity_fini(struct entity *entity);

void player_is_third_person(struct entity *entity);

#endif // VOXY_MAIN_GAME_PLAYER_H
