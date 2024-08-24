#include <voxy/scene/main_game/crafting/recipe.h>

bool recipe_check(const struct recipe *recipe, const struct item inputs[3][3])
{
  for(int i=0; i<3; ++i)
    for(int j=0; j<3; ++j)
    {
      if(recipe->inputs[i][j].id != inputs[i][j].id)
        return false;

      if((int)inputs[i][j].count < (int)recipe->inputs[i][j].count)
        return false;
    }

  return true;
}

void recipe_apply(const struct recipe *recipe, struct item inputs[3][3])
{
  for(int i=0; i<3; ++i)
    for(int j=0; j<3; ++j)
    {
      inputs[i][j].count -=  recipe->inputs[i][j].count;
      if(inputs[i][j].count == 0)
        inputs[i][j].id = ITEM_NONE;
    }
}

