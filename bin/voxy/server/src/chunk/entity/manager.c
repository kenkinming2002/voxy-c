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

void voxy_entity_manager_start(struct voxy_entity_database *entity_database, libnet_server_t server)
{
  int64_t *db_ids;
  if(voxy_entity_database_load_active(entity_database, &db_ids) != 0)
  {
    LOG_WARN("Failed to load active entitites at startup. Continuing anyway...");
    goto hack;
  }

  for(size_t i=0; i<arrlenu(db_ids); ++i)
  {
    const int64_t db_id = db_ids[i];

    struct voxy_entity entity;
    entity.db_id = db_id;

    if(voxy_entity_database_load(entity_database, &entity) != 0)
    {
      LOG_WARN("Failed to load entity from database. Continuing anyway...");
      continue;
    }

    voxy_entity_create(entity.db_id, entity.id, entity.position, entity.rotation, entity.opaque, server);
  }

  arrfree(db_ids);

hack:

  for(unsigned i=0; i<100; ++i)
    voxy_entity_spawn(entity_database, 1, fvec3(i, i, i), fvec3(i, i, i), NULL, server);
}

/// Create entity.
///
/// This takes care of allocating the entity and synchronizing the new state
/// over the network.
entity_handle_t voxy_entity_create(int64_t db_id, voxy_entity_id_t id, fvec3_t position, fvec3_t rotation, void *opaque, libnet_server_t server)
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
  voxy_entity_network_update_all(handle, entity, server);

  return handle;
}

/// Destroy entity.
///
/// This takes care of deallocating the entity and synchronizing the new state
/// over the network.
void voxy_entity_destroy(entity_handle_t handle, libnet_server_t server)
{
  voxy_entity_network_remove_all(handle, server);
  entity_free(handle);
}

static void load_entities(
    struct voxy_entity_database *entity_database,
    libnet_server_t server)
{
  profile_scope;

  size_t load_count = 0;
  for(ptrdiff_t i=0; i<hmlen(active_chunks); ++i)
  {
    ivec3_t position = active_chunks[i].key;
    if(hmgeti(loaded_chunks, position) == -1)
    {
      hmput(loaded_chunks, position, (struct empty){});

      int64_t *db_ids;
      if(voxy_entity_database_load_inactive(entity_database, position, &db_ids) != 0)
      {
        LOG_WARN("Failed to load inactive entitites for chunk at (%d, %d, %d). Continuing anyway...", position.x, position.y, position.z);
        continue;
      }

      for(size_t i=0; i<arrlenu(db_ids); ++i)
      {
        const int64_t db_id = db_ids[i];

        struct voxy_entity entity;
        entity.db_id = db_id;

        if(voxy_entity_database_load(entity_database, &entity) != 0)
        {
          LOG_WARN("Failed to load entity from database. Continuing anyway...");
          continue;
        }

        if(voxy_entity_database_uncommit(entity_database, entity.db_id) != 0)
        {
          LOG_WARN("Failed to uncommit entity from database. Continuing anyway...");
          continue;
        }

        voxy_entity_create(entity.db_id, entity.id, entity.position, entity.rotation, entity.opaque, server);
        load_count += 1;
      }

      arrfree(db_ids);
    }
  }

  if(load_count != 0)
    LOG_INFO("Entity Manager: Loaded %zu entities", load_count);
}

static void discard_entities(
    struct voxy_entity_database *entity_database,
    libnet_server_t server)
{
  profile_scope;

  size_t discard_count = 0;

  struct loaded_chunk *new_loaded_chunks = NULL;
  for(ptrdiff_t i=0; i<hmlen(loaded_chunks); ++i)
  {
    ivec3_t position = loaded_chunks[i].key;
    if(hmgeti(active_chunks, position) != -1)
      hmput(new_loaded_chunks, position, (struct empty){});
  }

  hmfree(loaded_chunks);
  loaded_chunks = new_loaded_chunks;

  struct voxy_entity *entities = entity_get_all();
  for(entity_handle_t handle=0; handle<arrlenu(entities); ++handle)
  {
    struct voxy_entity *entity = &entities[handle];
    if(!entity->alive)
      continue;

    const ivec3_t chunk_position = get_chunk_position_f(entity->position);
    if(hmgeti(loaded_chunks, chunk_position) != -1)
      continue;

    if(voxy_entity_database_commit(entity_database, entity->db_id, chunk_position) != 0) LOG_WARN("Failed to commit active entity for chunk at (%d, %d, %d). Continuing anyway...", chunk_position.x, chunk_position.y, chunk_position.z);
    voxy_entity_destroy(handle, server);

    discard_count += 1;
  }

  if(discard_count != 0)
    LOG_INFO("Entity Manager: Discarded %zu entities", discard_count);
}

static void flush_entities(
    struct voxy_entity_database *entity_database,
    libnet_server_t server)
{
  profile_scope;

  struct voxy_entity *entities = entity_get_all();
  for(entity_handle_t handle=0; handle<arrlenu(entities); ++handle)
  {
    struct voxy_entity *entity = &entities[handle];
    if(entity->alive)
    {
      voxy_entity_database_save(entity_database, entity);
      voxy_entity_network_update_all(handle, entity, server);
    }
  }
}

void voxy_entity_manager_update(struct voxy_entity_database *entity_database, libnet_server_t server)
{
  profile_scope;

  if(voxy_entity_database_begin_transaction(entity_database) != 0) LOG_ERROR("Failed to begin transaction");
  {
    load_entities(entity_database, server);
    flush_entities(entity_database, server);
    discard_entities(entity_database, server);
  }
  if(voxy_entity_database_end_transaction(entity_database) != 0) LOG_ERROR("Failed to begin transaction");
}

void voxy_entity_manager_on_client_connected(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct voxy_entity *entities = entity_get_all();
  for(entity_handle_t handle=0; handle<arrlenu(entities); ++handle)
  {
    const struct voxy_entity *entity = &entities[handle];
    if(entity->alive)
      voxy_entity_network_update(handle, entity, server, client_proxy);
  }
}

entity_handle_t voxy_entity_spawn(struct voxy_entity_database *entity_database, voxy_entity_id_t id, fvec3_t position, fvec3_t rotation, void *opaque, libnet_server_t server)
{
  // FIXME: We use 0 as placeholder for db_id, before we call
  //        voxy_entity_database_create() to hopefully get a real entity->db_id.
  //        This can have all sort of unexpected consequences if
  //        voxy_entity_database_create() failed.
  entity_handle_t handle = voxy_entity_create(0, id, position, rotation, opaque, server);
  if(voxy_entity_database_create(entity_database, voxy_entity_get(handle)) != 0) LOG_WARN("Failed to create entity in database. Continuing anyway...");
  return handle;
}

void voxy_entity_despawn(struct voxy_entity_database *entity_database, entity_handle_t handle, libnet_server_t server)
{
  if(voxy_entity_database_destroy(entity_database, voxy_entity_get(handle)) != 0) LOG_WARN("Failed to destroy entity in database. Continuing anyway...");
  voxy_entity_destroy(handle, server);
}

struct voxy_entity *voxy_entity_get(entity_handle_t handle)
{
  return entity_get(handle);
}

