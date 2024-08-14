#include <voxy/scene/main_game/update/chunk_database.h>
#include <voxy/scene/main_game/config.h>

#include <libcommon/core/log.h>
#include <libcommon/core/fs.h>
#include <libcommon/core/time.h>
#include <libcommon/core/serde.h>

#include <stdio.h>
#include <string.h>

#define LITERAL_LENGTH(x) (sizeof(x) - 1)
#define MAX(a,b) ({ typeof((a)) _a = (a), _b = (b); _a > _b ? _a : _b; })

#define INT_LENGTH 11

static int entity_deserialize(struct entity *entity, struct deserializer *deserializer)
{
  DESERIALIZE(deserializer, entity->id);

  DESERIALIZE(deserializer, entity->position);
  DESERIALIZE(deserializer, entity->rotation);
  DESERIALIZE(deserializer, entity->velocity);

  DESERIALIZE(deserializer, entity->health);
  DESERIALIZE(deserializer, entity->max_health);

  DESERIALIZE(deserializer, entity->max_height);
  DESERIALIZE(deserializer, entity->grounded);

  entity->remove = false;

  const struct entity_info *entity_info = query_entity_info(entity->id);;
  if(entity_info->deserialize)
    entity_info->deserialize(entity, deserializer);

  return 0;
}

static int entity_serialize(const struct entity *entity, struct serializer *serializer)
{
  SERIALIZE(serializer, entity->id);

  SERIALIZE(serializer, entity->position);
  SERIALIZE(serializer, entity->rotation);
  SERIALIZE(serializer, entity->velocity);

  SERIALIZE(serializer, entity->health);
  SERIALIZE(serializer, entity->max_health);

  SERIALIZE(serializer, entity->max_height);
  SERIALIZE(serializer, entity->grounded);

  const struct entity_info *entity_info = query_entity_info(entity->id);;
  if(entity_info->serialize)
    entity_info->serialize(entity, serializer);

  return 0;
}

static int entities_deserialize(struct entities *entities, struct deserializer *deserializer)
{
  size_t item_count;
  DESERIALIZE(deserializer, item_count);

  entities_init(entities);
  for(size_t i=0; i<item_count; ++i)
  {
    struct entity entity;
    if(entity_deserialize(&entity, deserializer) != 0)
    {
      entities_fini(entities);
      return -1;
    }
    entities_append(entities, entity);
  }

  return 0;
}

static int entities_serialize(struct entities *entities, struct serializer *serializer)
{
  SERIALIZE(serializer, entities->item_count);
  for(size_t i=0; i<entities->item_count; ++i)
    entity_serialize(&entities->items[i], serializer);

  return 0;
}

static int chunk_deserialize(struct chunk *chunk, struct deserializer *deserializer)
{
  DESERIALIZE(deserializer, chunk->block_ids);
  DESERIALIZE(deserializer, chunk->block_light_levels);

  if(entities_deserialize(&chunk->entities, deserializer) != 0)
    return -1;

  entities_init(&chunk->new_entities);
  return 0;
}

static int chunk_serialize(struct chunk *chunk, struct serializer *serializer)
{
  SERIALIZE(serializer, chunk->block_ids);
  SERIALIZE(serializer, chunk->block_light_levels);

  if(entities_serialize(&chunk->entities, serializer) != 0)
    return -1;

  return 0;
}

struct chunk *load_chunk(ivec3_t position)
{
  struct chunk *chunk = malloc(sizeof *chunk);
  chunk->position = position;
  chunk->dirty = false;
  chunk->last_save_time = get_time();

  char buffer[LITERAL_LENGTH(WORLD_DIRPATH) + (1 + LITERAL_LENGTH("chunks")) + (1 + INT_LENGTH) * 3 + 1 + LITERAL_LENGTH("data") + 1];
  snprintf(buffer, sizeof buffer, "%s%c%s%c%d%c%d%c%d%cdata", WORLD_DIRPATH, DIRECTORY_SEPARATOR, "chunks", DIRECTORY_SEPARATOR, chunk->position.x, DIRECTORY_SEPARATOR, chunk->position.y, DIRECTORY_SEPARATOR, chunk->position.z, DIRECTORY_SEPARATOR);

  struct deserializer deserializer;
  if(deserializer_init(&deserializer, buffer) != 0)
    return NULL;

  if(chunk_deserialize(chunk, &deserializer) != 0)
  {
    deserializer_fini(&deserializer);
    return NULL;
  }

  deserializer_fini(&deserializer);
  return chunk;
}

bool save_chunk(struct chunk *chunk)
{
  char buffer[LITERAL_LENGTH(WORLD_DIRPATH) + (1 + LITERAL_LENGTH("chunks")) + (1 + INT_LENGTH) * 3 + 1 + LITERAL_LENGTH("data") + 1];
  snprintf(buffer, sizeof buffer, "%s%c%s%c%d%c%d%c%d%cdata", WORLD_DIRPATH, DIRECTORY_SEPARATOR, "chunks", DIRECTORY_SEPARATOR, chunk->position.x, DIRECTORY_SEPARATOR, chunk->position.y, DIRECTORY_SEPARATOR, chunk->position.z, DIRECTORY_SEPARATOR);

  struct serializer serializer;
  if(serializer_init(&serializer, buffer) != 0)
    return false;

  if(chunk_serialize(chunk, &serializer) != 0)
  {
    serializer_fini(&serializer);
    return false;
  }

  serializer_fini(&serializer);
  return true;
}

