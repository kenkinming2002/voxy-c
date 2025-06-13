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
  struct voxy_entity *entities;
  voxy_entity_database_load_active(&entities);

  for(size_t i=0; i<arrlenu(entities); ++i)
  {
    entity_handle_t handle = entity_alloc();
    struct voxy_entity *entity = entity_get(handle);

    *entity = entities[i];
    voxy_entity_network_update_all(handle, entity);
  }

  arrfree(entities);

  for(unsigned i=0; i<100; ++i)
    voxy_entity_spawn(1, fvec3(i, i, i), fvec3(i, i, i), NULL);
}

struct major_chunk_statistic
{
  size_t load_count;
  size_t load_entity_count;
};

static bool block_manager_major_chunk_iterate(ivec3_t chunk_position, void *data)
{
  struct major_chunk_statistic *statistic = data;

  if(hmgetp_null(loaded_chunks, chunk_position))
    return true;

  struct voxy_entity *entities;
  voxy_entity_database_load_inactive(chunk_position, &entities);

  for(size_t i=0; i<arrlenu(entities); ++i)
  {
    entity_handle_t handle = entity_alloc();
    struct voxy_entity *entity = entity_get(handle);

    *entity = entities[i];
    voxy_entity_network_update_all(handle, entity);
  }

  statistic->load_entity_count += arrlenu(entities);
  arrfree(entities);

  statistic->load_count += 1;
  hmput(loaded_chunks, chunk_position, (struct empty){});

  return true;
}


void voxy_entity_manager_update(void)
{
  profile_scope;

  voxy_entity_database_begin_transaction();

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
        voxy_entity_database_update(entity);
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
          voxy_entity_database_unload(entity);
          voxy_entity_network_remove_all(handle);
          entity_free(handle);
          unload_entity_count += 1;
        }
      }
    }

    if(unload_count != 0)
      LOG_INFO("Entity Manager: Unloaded %zu entities from %zu chunks", unload_entity_count, unload_count);
  }

  voxy_entity_database_end_transaction();
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
  entity_handle_t handle = entity_alloc();
  struct voxy_entity *entity = entity_get(handle);
  entity->id = id;
  entity->position = position;
  entity->rotation = rotation;
  entity->opaque = opaque;
  voxy_entity_database_create(entity);
  voxy_entity_network_update_all(handle, entity);
  return handle;
}

void voxy_entity_despawn(entity_handle_t handle)
{
  struct voxy_entity *entity = entity_get(handle);
  voxy_entity_database_destroy(entity);
  voxy_entity_network_remove_all(handle);
  entity_free(handle);
}

struct voxy_entity *voxy_entity_get(entity_handle_t handle)
{
  return entity_get(handle);
}

