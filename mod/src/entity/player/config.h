#ifndef ENTITY_PLAYER_CONFIG_H
#define ENTITY_PLAYER_CONFIG_H

#define PLAYER_INVENTORY_SIZE_HORIZONTAL 9
#define PLAYER_INVENTORY_SIZE_VERTICAL 5

#define PLAYER_HOTBAR_SIZE 9

#define PLAYER_UI_TEXT_SIZE 28
#define PLAYER_UI_TEXT_SIZE_ITEM_COUNT 20

#define PLAYER_INVENTORY_UI_COLOR_BACKGROUND fvec4(0.9f, 0.9f, 0.9f, 1.0f)
#define PLAYER_INVENTORY_UI_COLOR_HOVER fvec4(0.4f,  0.8f,  0.2f,  0.8f)
#define PLAYER_INVENTORY_UI_COLOR_DEFAULT fvec4(0.95f, 0.95f, 0.95f, 0.7f)

#define PLAYER_HOTBAR_UI_COLOR_BACKGROUND fvec4(0.9f, 0.9f, 0.9f, 0.3f)
#define PLAYER_HOTBAR_UI_COLOR_SELECTED fvec4(0.95f, 0.75f, 0.75f, 0.8f)
#define PLAYER_HOTBAR_UI_COLOR_HOVER fvec4(0.4f,  0.8f,  0.2f,  0.8f)
#define PLAYER_HOTBAR_UI_COLOR_DEFAULT fvec4(0.95f, 0.95f, 0.95f, 0.7f)

#define PLAYER_PAN_SPEED 0.002f

#define PLAYER_MOVE_SPEED_GROUND 8.0f
#define PLAYER_MOVE_SPEED_AIR 3.0f

#define PLAYER_ACTION_COOLDOWN 0.5f

#define PLAYER_JUMP_STRENGTH 8.0f

#define PLAYER_DIG_SPEED 1.0f

#define PLAYER_CHUNK_LOAD_DISTANCE 8

#endif // ENTITY_PLAYER_CONFIG_H
