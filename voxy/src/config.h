#ifndef VOXY_CONFIG_H
#define VOXY_CONFIG_H

#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "voxy"

#define RESOURCE_PACK_FILEPATH "resource_pack/resource_pack.so"

#define UI_HOTBAR_COLOR_BACKGROUND fvec4(0.9f, 0.9f, 0.9f, 0.3f)
#define UI_HOTBAR_COLOR_SELECTED   fvec4(0.95f, 0.75f, 0.75f, 0.8f)
#define UI_HOTBAR_COLOR_HOVER      fvec4(0.4f,  0.8f,  0.2f,  0.8f)
#define UI_HOTBAR_COLOR_DEFAULT    fvec4(0.95f, 0.95f, 0.95f, 0.7f)

#define UI_INVENTORY_COLOR_BACKGROUND fvec4(0.9f, 0.9f, 0.9f, 1.0f)
#define UI_INVENTORY_COLOR_HOVER      fvec4(0.4f,  0.8f,  0.2f,  0.8f)
#define UI_INVENTORY_COLOR_DEFAULT    fvec4(0.95f, 0.95f, 0.95f, 0.7f)

#define UI_TEXT_SIZE 28
#define UI_TEXT_SIZE_ITEM_COUNT 20

#define GENERATOR_DISTANCE 8

#define RENDERER_LOAD_DISTANCE   4
#define RENDERER_UNLOAD_DISTANCE 64

#define PLAYER_MOVE_SPEED 20.0f
#define PLAYER_PAN_SPEED  0.002f
#define PLAYER_ACTION_COOLDOWN 0.5f

#define PLAYER_EYE_HEIGHT 0.9f
#define PLAYER_DIMENSION fvec3(0.9f, 0.9f, 2.0f)

#define PHYSICS_GRAVITY 9.8f
#define PHYSICS_DRAG 1.0f

#endif // VOXY_CONFIG_H
