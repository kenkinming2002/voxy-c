#ifndef MAIN_GAME_PLAYER_H
#define MAIN_GAME_PLAYER_H

#include <stdbool.h>

struct player;
struct entity;

struct player *player_get(void);

struct entity *player_as_entity(struct player *player);
bool player_third_person(struct player *player);

void update_spawn_player(void);

#endif // MAIN_GAME_PLAYER_H
