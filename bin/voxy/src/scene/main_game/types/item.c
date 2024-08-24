#include <voxy/scene/main_game/types/item.h>

void item_merge(struct item* target, struct item *source)
{
  if(target->id == source->id)
  {
    const unsigned capacity = ITEM_MAX_STACK - target->count;
    const unsigned available = source->count;

    const unsigned count = capacity < available ? capacity : available;
    target->count += count;
    source->count -= count;

    if(source->count == 0)
      source->id = ITEM_NONE;
  }
  else if(target->id == ITEM_NONE)
  {
    *target = *source;
    source->id = ITEM_NONE;
    source->count = 0;
  }
}
