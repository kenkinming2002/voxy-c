#include "manager.h"

#include "allocator.h"
#include "database.h"
#include "network.h"

#include "chunk/coordinates.h"
#include "chunk/manager.h"

#include <libserde/serializer.h>

#include <libcore/log.h>
#include <libcore/profile.h>

#include <stb_ds.h>

#include <assert.h>

struct loaded_chunk
{
  ivec3_t key;
  struct empty value;
};

static struct loaded_chunk *loaded_chunks;

void voxy_entity_manager_start(void)
{
  int64_t *db_ids;
  if(voxy_entity_database_load_active(&db_ids) != 0)
  {
    LOG_WARN("Failed to load active entitites at startup. Continuing anyway...");
    goto hack;
  }

  for(size_t i=0; i<arrlenu(db_ids); ++i)
  {
    const int64_t db_id = db_ids[i];

    struct voxy_entity entity;
    entity.db_id = db_id;

    if(voxy_entity_database_load(&entity) != 0)
    {
      LOG_WARN("Failed to load entity from database. Continuing anyway...");
      continue;
    }

    voxy_entity_create(entity.db_id, entity.id, entity.position, entity.rotation, entity.opaque);
  }

  arrfree(db_ids);

hack:

  for(unsigned i=0; i<100; ++i)
    voxy_entity_spawn(1, fvec3(i, i, i), fvec3(i, i, i), NULL);
}

/// Create entity.
///
/// This takes care of allocating the entity and synchronizing the new state
/// over the network.
entity_handle_t voxy_entity_create(int64_t db_id, voxy_entity_id_t id, fvec3_t position, fvec3_t rotation, void *opaque)
{
  entity_handle_t handle = entity_alloc();

  struct voxy_entity *entity = voxy_entity_get(handle);
  entity->db_id = db_id;
  entity->id = id;
  entity->position = position;
  entity->rotation = rotation;
  entity->velocity = fvec3_zero();
  entity->grounded = false;
  entity->opaque = opaque;
  voxy_entity_network_update_all(handle, entity);

  return handle;
}

/// Destroy entity.
///
/// This takes care of deallocating the entity and synchronizing the new state
/// over the network.
void voxy_entity_destroy(entity_handle_t handle)
{
  voxy_entity_network_remove_all(handle);
  entity_free(handle);
}

struct major_chunk_statistic
{
  size_t load_count;
  size_t load_entity_count;
};

static void block_manager_major_chunk_iterate(ivec3_t chunk_position, void *data)
{
  struct major_chunk_statistic *statistic = data;

  if(hmgetp_null(loaded_chunks, chunk_position))
    return;

  int64_t *db_ids;
  if(voxy_entity_database_load_inactive(chunk_position, &db_ids) != 0)
  {
    LOG_WARN("Failed to load inactive entitites for chunk at (%d, %d, %d). Continuing anyway...", chunk_position.x, chunk_position.y, chunk_position.z);
    return;
  }

  for(size_t i=0; i<arrlenu(db_ids); ++i)
  {
    const int64_t db_id = db_ids[i];

    struct voxy_entity entity;
    entity.db_id = db_id;

    if(voxy_entity_database_load(&entity) != 0)
    {
      LOG_WARN("Failed to load entity from database. Continuing anyway...");
      continue;
    }

    if(voxy_entity_database_uncommit(entity.db_id) != 0)
    {
      LOG_WARN("Failed to uncommit entity from database. Continuing anyway...");
      continue;
    }

    voxy_entity_create(entity.db_id, entity.id, entity.position, entity.rotation, entity.opaque);
    statistic->load_entity_count += 1;
  }

  arrfree(db_ids);

  hmput(loaded_chunks, chunk_position, (struct empty){});
  statistic->load_count += 1;
}


void voxy_entity_manager_update(void)
{
  profile_scope;

  if(voxy_entity_database_begin_transaction() != 0)
    LOG_ERROR("Failed to begin transaction");

  // Iterate major chunks and try to load new entities.
  {
    profile_scope;

    struct major_chunk_statistic statistic = {0};
    iterate_major_chunk(&block_manager_major_chunk_iterate, &statistic);

    if(statistic.load_count != 0)
      LOG_INFO("Entity Manager: Loaded %zu entities from %zu chunks", statistic.load_entity_count, statistic.load_count);
  }

  // Iterate existing entities and try to flush them.
  {
    profile_scope;

    struct voxy_entity *entities = entity_get_all();
    for(entity_handle_t handle=0; handle<arrlenu(entities); ++handle)
    {
      struct voxy_entity *entity = &entities[handle];
      if(entity->alive)
      {
        voxy_entity_database_save(entity);
        voxy_entity_network_update_all(handle, entity);
      }
    }
  }

  // Iterate existing entities and try to unload them.
  {
    profile_scope;

    size_t unload_count = 0;
    size_t unload_entity_count = 0;

    struct loaded_chunk *new_loaded_chunks = NULL;
    for(ptrdiff_t i=0; i<hmlen(loaded_chunks); ++i)
    {
      ivec3_t chunk_position = loaded_chunks[i].key;
      if(is_minor_chunk(chunk_position))
      {
        hmput(new_loaded_chunks, chunk_position, (struct empty){});
        continue;
      }

      unload_count += 1;
    }

    hmfree(loaded_chunks);
    loaded_chunks = new_loaded_chunks;

    struct voxy_entity *entities = entity_get_all();
    for(entity_handle_t handle=0; handle<arrlenu(entities); ++handle)
    {
      struct voxy_entity *entity = &entities[handle];
      if(entity->alive)
      {
        const ivec3_t chunk_position = get_chunk_position_f(entity->position);
        if(!hmgetp_null(loaded_chunks, chunk_position))
        {
          if(voxy_entity_database_commit(entity->db_id, chunk_position) != 0) LOG_WARN("Failed to commit active entity for chunk at (%d, %d, %d). Continuing anyway...", chunk_position.x, chunk_position.y, chunk_position.z);
          voxy_entity_destroy(handle);
          unload_entity_count += 1;
        }
      }
    }

    if(unload_count != 0)
      LOG_INFO("Entity Manager: Unloaded %zu entities from %zu chunks", unload_entity_count, unload_count);
  }

  if(voxy_entity_database_end_transaction() != 0)
    LOG_ERROR("Failed to begin transaction");
}

void voxy_entity_manager_on_client_connected(libnet_client_proxy_t client_proxy)
{
  struct voxy_entity *entities = entity_get_all();
  for(entity_handle_t handle=0; handle<arrlenu(entities); ++handle)
  {
    const struct voxy_entity *entity = &entities[handle];
    if(entity->alive)
      voxy_entity_network_update(handle, entity, client_proxy);
  }
}

entity_handle_t voxy_entity_spawn(voxy_entity_id_t id, fvec3_t position, fvec3_t rotation, void *opaque)
{
  // FIXME: We use 0 as placeholder for db_id, before we call
  //        voxy_entity_database_create() to hopefully get a real entity->db_id.
  //        This can have all sort of unexpected consequences if
  //        voxy_entity_database_create() failed.
  entity_handle_t handle = voxy_entity_create(0, id, position, rotation, opaque);
  if(voxy_entity_database_create(voxy_entity_get(handle)) != 0) LOG_WARN("Failed to create entity in database. Continuing anyway...");
  return handle;
}

void voxy_entity_despawn(entity_handle_t handle)
{
  if(voxy_entity_database_destroy(voxy_entity_get(handle)) != 0) LOG_WARN("Failed to destroy entity in database. Continuing anyway...");
  voxy_entity_destroy(handle);
}

struct voxy_entity *voxy_entity_get(entity_handle_t handle)
{
  return entity_get(handle);
}

