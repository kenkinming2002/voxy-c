#include "manager.h"

#include "database.h"
#include "network.h"

#include "chunk/coordinates.h"

#include <libserde/serializer.h>

#include <libcommon/core/log.h>
#include <libcommon/core/profile.h>

#include <assert.h>

void voxy_entity_manager_init(struct voxy_entity_manager *entity_manager)
{
  entity_allocator_init(&entity_manager->allocator);
  ivec3_hash_table_init(&entity_manager->loaded_chunks);
}

void voxy_entity_manager_fini(struct voxy_entity_manager *entity_manager)
{
  ivec3_hash_table_dispose(&entity_manager->loaded_chunks);
  entity_allocator_fini(&entity_manager->allocator);
}

void voxy_entity_manager_start(struct voxy_entity_manager *entity_manager, struct voxy_entity_registry *entity_registry, struct voxy_entity_database *entity_database, libnet_server_t server)
{
  struct db_ids db_ids = {0};
  if(voxy_entity_database_load_active(entity_database, &db_ids) != 0)
  {
    LOG_WARN("Failed to load active entitites at startup. Continuing anyway...");
    goto hack;
  }

  for(size_t i=0; i<db_ids.item_count; ++i)
  {
    const int64_t db_id = db_ids.items[i];

    struct voxy_entity entity;
    entity.db_id = db_id;

    if(voxy_entity_database_load(entity_database, entity_registry, &entity) != 0)
    {
      LOG_WARN("Failed to load entity from database. Continuing anyway...");
      continue;
    }

    voxy_entity_manager_create_entity(entity_manager, entity.db_id, entity.id, entity.position, entity.rotation, entity.opaque, server);
  }

  DYNAMIC_ARRAY_CLEAR(db_ids);

hack:

  for(unsigned i=0; i<100; ++i)
    voxy_entity_manager_spawn(entity_manager, entity_registry, entity_database, 1, fvec3(i, i, i), fvec3(i, i, i), NULL, server);
}

/// Create entity.
///
/// This takes care of allocating the entity and synchronizing the new state
/// over the network.
entity_handle_t voxy_entity_manager_create_entity(struct voxy_entity_manager *entity_manager, int64_t db_id, voxy_entity_id_t id, fvec3_t position, fvec3_t rotation, void *opaque, libnet_server_t server)
{
  entity_handle_t handle = entity_allocator_alloc(&entity_manager->allocator);

  struct voxy_entity *entity = voxy_entity_manager_get(entity_manager, handle);
  entity->db_id = db_id;
  entity->id = id;
  entity->position = position;
  entity->rotation = rotation;
  entity->velocity = fvec3_zero();
  entity->grounded = false;
  entity->opaque = opaque;
  voxy_entity_network_update_all(handle, entity, server);

  return handle;
}

/// Destroy entity.
///
/// This takes care of deallocating the entity and synchronizing the new state
/// over the network.
void voxy_entity_manager_destroy_entity(struct voxy_entity_manager *entity_manager, struct voxy_entity_registry *entity_registry, entity_handle_t handle, libnet_server_t server)
{
  voxy_entity_network_remove_all(handle, server);
  entity_allocator_free(&entity_manager->allocator, entity_registry, handle);
}

static void load_entities(
    struct voxy_entity_manager *entity_manager,
    struct voxy_entity_registry *entity_registry,
    struct voxy_entity_database *entity_database,
    struct voxy_chunk_manager *chunk_manager,
    libnet_server_t server)
{
  profile_begin();

  size_t load_count = 0;

  struct ivec3_node *active_chunk;
  struct ivec3_node *loaded_chunk;
  SC_HASH_TABLE_FOREACH(chunk_manager->active_chunks, active_chunk)
    if(!(loaded_chunk = ivec3_hash_table_lookup(&entity_manager->loaded_chunks, active_chunk->key)))
    {
      loaded_chunk = malloc(sizeof *loaded_chunk);
      loaded_chunk->key = active_chunk->key;
      ivec3_hash_table_insert(&entity_manager->loaded_chunks, loaded_chunk);

      struct db_ids db_ids = {0};
      if(voxy_entity_database_load_inactive(entity_database, loaded_chunk->key, &db_ids) != 0)
      {
        LOG_WARN("Failed to load inactive entitites for chunk at (%d, %d, %d). Continuing anyway...", active_chunk->key.x, active_chunk->key.y, active_chunk->key.z);
        continue;
      }

      for(size_t i=0; i<db_ids.item_count; ++i)
      {
        const int64_t db_id = db_ids.items[i];

        struct voxy_entity entity;
        entity.db_id = db_id;

        if(voxy_entity_database_load(entity_database, entity_registry, &entity) != 0)
        {
          LOG_WARN("Failed to load entity from database. Continuing anyway...");
          continue;
        }

        if(voxy_entity_database_uncommit(entity_database, entity.db_id) != 0)
        {
          LOG_WARN("Failed to uncommit entity from database. Continuing anyway...");
          continue;
        }

        voxy_entity_manager_create_entity(entity_manager, entity.db_id, entity.id, entity.position, entity.rotation, entity.opaque, server);
        load_count += 1;
      }

      DYNAMIC_ARRAY_CLEAR(db_ids);
    }

  if(load_count != 0)
    LOG_INFO("Entity Manager: Loaded %zu entities", load_count);

  profile_end();
}

static void discard_entities(
    struct voxy_entity_manager *entity_manager,
    struct voxy_entity_registry *entity_registry,
    struct voxy_entity_database *entity_database,
    struct voxy_chunk_manager *chunk_manager,
    libnet_server_t server)
{
  profile_begin();

  size_t discard_count = 0;

  for(size_t i=0; i<entity_manager->loaded_chunks.bucket_count; ++i)
  {
    struct ivec3_node **node = &entity_manager->loaded_chunks.buckets[i].head;
    while(*node)
      if(!ivec3_hash_table_lookup(&chunk_manager->active_chunks, (*node)->key))
      {
        struct ivec3_node *old_node = *node;
        *node = (*node)->next;
        free(old_node);
        entity_manager->loaded_chunks.load -= 1;
      }
      else
        node = &(*node)->next;
  }

  for(entity_handle_t handle=0; handle<entity_manager->allocator.entities.item_count; ++handle)
  {
    struct voxy_entity *entity = &entity_manager->allocator.entities.items[handle];
    if(!entity->alive)
      continue;

    const ivec3_t chunk_position = get_chunk_position_f(entity->position);
    if(ivec3_hash_table_lookup(&entity_manager->loaded_chunks, chunk_position))
      continue;

    if(voxy_entity_database_commit(entity_database, entity->db_id, chunk_position) != 0) LOG_WARN("Failed to commit active entity for chunk at (%d, %d, %d). Continuing anyway...", chunk_position.x, chunk_position.y, chunk_position.z);
    voxy_entity_manager_destroy_entity(entity_manager, entity_registry, handle, server);

    discard_count += 1;
  }

  if(discard_count != 0)
    LOG_INFO("Entity Manager: Discarded %zu entities", discard_count);

  profile_end();
}

static void flush_entities(
    struct voxy_entity_manager *entity_manager,
    struct voxy_entity_registry *entity_registry,
    struct voxy_entity_database *entity_database,
    libnet_server_t server)
{
  profile_begin();

  for(entity_handle_t handle=0; handle<entity_manager->allocator.entities.item_count; ++handle)
  {
    struct voxy_entity *entity = &entity_manager->allocator.entities.items[handle];
    if(entity->alive)
    {
      voxy_entity_database_save(entity_database, entity_registry, entity);
      voxy_entity_network_update_all(handle, entity, server);
    }
  }

  profile_end();
}

void voxy_entity_manager_update(struct voxy_entity_manager *entity_manager, struct voxy_entity_registry *entity_registry, struct voxy_entity_database *entity_database, struct voxy_chunk_manager *chunk_manager, libnet_server_t server)
{
  profile_begin();

  if(voxy_entity_database_begin_transaction(entity_database) != 0) LOG_ERROR("Failed to begin transaction");
  {
    load_entities(entity_manager, entity_registry, entity_database, chunk_manager, server);
    flush_entities(entity_manager, entity_registry, entity_database, server);
    discard_entities(entity_manager, entity_registry, entity_database, chunk_manager, server);
  }
  if(voxy_entity_database_end_transaction(entity_database) != 0) LOG_ERROR("Failed to begin transaction");

  profile_end();
}

void voxy_entity_manager_on_client_connected(struct voxy_entity_manager *entity_manager, libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  for(entity_handle_t handle=0; handle<entity_manager->allocator.entities.item_count; ++handle)
  {
    const struct voxy_entity *entity = &entity_manager->allocator.entities.items[handle];
    if(entity->alive)
      voxy_entity_network_update(handle, entity, server, client_proxy);
  }
}

entity_handle_t voxy_entity_manager_spawn(struct voxy_entity_manager *entity_manager, struct voxy_entity_registry *entity_registry, struct voxy_entity_database *entity_database, voxy_entity_id_t id, fvec3_t position, fvec3_t rotation, void *opaque, libnet_server_t server)
{
  // FIXME: We use 0 as placeholder for db_id, before we call
  //        voxy_entity_database_create() to hopefully get a real entity->db_id.
  //        This can have all sort of unexpected consequences if
  //        voxy_entity_database_create() failed.
  entity_handle_t handle = voxy_entity_manager_create_entity(entity_manager, 0, id, position, rotation, opaque, server);
  if(voxy_entity_database_create(entity_database, entity_registry, voxy_entity_manager_get(entity_manager, handle)) != 0) LOG_WARN("Failed to create entity in database. Continuing anyway...");
  return handle;
}

void voxy_entity_manager_despawn(struct voxy_entity_manager *entity_manager, struct voxy_entity_registry *entity_registry, struct voxy_entity_database *entity_database, entity_handle_t handle, libnet_server_t server)
{
  if(voxy_entity_database_destroy(entity_database, voxy_entity_manager_get(entity_manager, handle)) != 0) LOG_WARN("Failed to destroy entity in database. Continuing anyway...");
  voxy_entity_manager_destroy_entity(entity_manager, entity_registry, handle, server);
}

struct voxy_entity *voxy_entity_manager_get(struct voxy_entity_manager *entity_manager, entity_handle_t handle)
{
  return entity_allocator_get(&entity_manager->allocator, handle);
}

