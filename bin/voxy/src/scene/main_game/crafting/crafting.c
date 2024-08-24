#include <voxy/scene/main_game/crafting/crafting.h>

#include <libcommon/utils/dynamic_array.h>

static DYNAMIC_ARRAY_DECLARE(recipes, struct recipe);

void crafting_add_recipe(struct recipe recipe)
{
  DYNAMIC_ARRAY_APPEND(recipes, recipe);
}

const struct recipe *crafting_find_recipe(const struct item inputs[3][3])
{
  for(size_t i=0; i<recipes.item_count; ++i)
    if(recipe_check(&recipes.items[i], inputs))
      return &recipes.items[i];

  return NULL;
}
