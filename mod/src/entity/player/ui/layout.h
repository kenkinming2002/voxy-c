#ifndef ENTITY_PLAYER_UI_LAYOUT_H
#define ENTITY_PLAYER_UI_LAYOUT_H

#include <libcommon/math/vector.h>
#include <libcommon/ui/layout.h>

struct player_ui_layout
{
  struct ui_rect hand;

  struct ui_grid hot_bar;
  struct ui_grid health_bar;

  struct ui_grid inventory;

  struct ui_grid crafting_inputs;
  struct ui_grid crafting_output;

  struct ui_rect cursor;

  float selected_item_y;
  float selected_item_height;

  float target_block_y;
  float target_block_height;
};

struct player_ui_layout compute_player_ui_layout(void);

#endif // ENTITY_PLAYER_UI_LAYOUT_H
