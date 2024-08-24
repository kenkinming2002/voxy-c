#ifndef VOXY_SCENE_MAIN_GAME_CRAFTING_RECIPE_H
#define VOXY_SCENE_MAIN_GAME_CRAFTING_RECIPE_H

#include <voxy/scene/main_game/types/registry.h>
#include <voxy/scene/main_game/types/item.h>

#include <stdbool.h>

struct recipe
{
  struct item inputs[3][3];
  struct item output;
};

// Check if a recipe can be applied to inputs.
bool recipe_check(const struct recipe *recipe, const struct item inputs[3][3]);

/// Apply and recipe, returning its output and modifying inputs to account for
/// items consumed.
///
/// Precondition: recipe_check(recipe, inputs)
void recipe_apply(const struct recipe *recipe, struct item inputs[3][3]);

#endif // VOXY_SCENE_MAIN_GAME_CRAFTING_RECIPE_H
