#include <voxy/scene/main_game/update/chunk_database.h>
#include <voxy/scene/main_game/config.h>

#include <voxy/core/log.h>
#include <voxy/core/fs.h>
#include <voxy/core/time.h>

#include <stdio.h>
#include <string.h>

#define LITERAL_LENGTH(x) (sizeof(x) - 1)
#define MAX(a,b) ({ typeof((a)) _a = (a), _b = (b); _a > _b ? _a : _b; })

#define INT_LENGTH 11

struct chunk_data *load_chunk_data(ivec3_t position)
{
  struct chunk_data *chunk_data = malloc(sizeof *chunk_data);
  chunk_data->dirty = false;
  chunk_data->last_save_time = get_time();

  char buffer[LITERAL_LENGTH(WORLD_DIRPATH) + (1 + LITERAL_LENGTH("chunks")) + (1 + INT_LENGTH) * 3 + 1 + MAX(LITERAL_LENGTH("blocks"), LITERAL_LENGTH("entities")) + 1];
  int n = snprintf(buffer, sizeof buffer, "%s%c%s%c%d%c%d%c%d", WORLD_DIRPATH, DIRECTORY_SEPARATOR, "chunks", DIRECTORY_SEPARATOR, position.x, DIRECTORY_SEPARATOR, position.y, DIRECTORY_SEPARATOR, position.z);

  buffer[n] = DIRECTORY_SEPARATOR;

  strcpy(&buffer[n+1], "blocks");
  {
    FILE *f = fopen(buffer, "rb");
    if(!f)
      return NULL;

    if(fread(&chunk_data->block_ids, sizeof chunk_data->block_ids, 1, f) != 1)
    {
      fclose(f);
      return NULL;
    }

    if(fread(&chunk_data->block_light_levels, sizeof chunk_data->block_light_levels, 1, f) != 1)
    {
      fclose(f);
      return NULL;
    }

    fclose(f);
  }

  strcpy(&buffer[n+1], "entities");
  {
    FILE *f = fopen(buffer, "rb");
    if(!f)
      return NULL;

    DYNAMIC_ARRAY_INIT(chunk_data->entities);
    DYNAMIC_ARRAY_INIT(chunk_data->new_entities);

    for(;;)
    {
      struct entity entity = {0};
      if(fread(&entity, offsetof(struct entity, opaque), 1, f) != 1)
        break;

      const struct entity_info *entity_info = query_entity_info(entity.id);
      if(entity_info->on_load && !entity_info->on_load(&entity, f))
      {
        fclose(f);
        return false;
      }

      DYNAMIC_ARRAY_APPEND(chunk_data->entities, entity);
    }

    if(!feof(f))
    {
      fclose(f);
      return false;
    }

    fclose(f);
  }

  return chunk_data;
}

bool save_chunk_data(ivec3_t position, struct chunk_data *chunk_data)
{
  char buffer[LITERAL_LENGTH(WORLD_DIRPATH) + (1 + INT_LENGTH) * 3 + 1 + MAX(LITERAL_LENGTH("blocks"), LITERAL_LENGTH("entities")) + 1];
  int n = snprintf(buffer, sizeof buffer, "%s%c%s%c%d%c%d%c%d", WORLD_DIRPATH, DIRECTORY_SEPARATOR, "chunks", DIRECTORY_SEPARATOR, position.x, DIRECTORY_SEPARATOR, position.y, DIRECTORY_SEPARATOR, position.z);
  mkdir_recursive(buffer);

  buffer[n] = DIRECTORY_SEPARATOR;

  strcpy(&buffer[n+1], "blocks");
  {
    FILE *f = fopen(buffer, "wb");
    if(!f)
      return false;

    if(fwrite(&chunk_data->block_ids, sizeof chunk_data->block_ids, 1, f) != 1)
    {
      fclose(f);
      return false;
    }

    if(fwrite(&chunk_data->block_light_levels, sizeof chunk_data->block_light_levels, 1, f) != 1)
    {
      fclose(f);
      return false;
    }

    fclose(f);
  }

  strcpy(&buffer[n+1], "entities");
  {
    FILE *f = fopen(buffer, "wb");
    if(!f)
      return false;

    for(size_t i=0; i<chunk_data->entities.item_count; ++i)
    {
      const struct entity entity = chunk_data->entities.items[i];
      const struct entity_info *entity_info = query_entity_info(entity.id);

      if(fwrite(&entity, offsetof(struct entity, opaque), 1, f) != 1)
      {
        fclose(f);
        return false;
      }

      if(entity_info->on_save && !entity_info->on_save(&entity, f))
      {
        fclose(f);
        return false;
      }
    }

    fclose(f);
  }

  chunk_data->last_save_time = get_time();
  return true;
}

