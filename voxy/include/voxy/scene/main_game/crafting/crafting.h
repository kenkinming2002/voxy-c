#ifndef VOXY_SCENE_MAIN_GAME_CRAFTING_CRAFTING_H
#define VOXY_SCENE_MAIN_GAME_CRAFTING_CRAFTING_H

#include "recipe.h"

/// Add an recipe to the crafting system.
void crafting_add_recipe(struct recipe recipe);

/// Find the first recipe that can be applied to inputs, returning NULL if none
/// could be found. The output
const struct recipe *crafting_find_recipe(const struct item inputs[3][3]);

#endif // VOXY_SCENE_MAIN_GAME_CRAFTING_CRAFTING_H
