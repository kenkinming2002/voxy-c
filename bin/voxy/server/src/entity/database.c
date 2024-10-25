#include "database.h"

#include "chunk/coordinates.h"

#include <libserde/serializer.h>
#include <libserde/deserializer.h>

#include <libcommon/core/fs.h>

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

#define ENTITIES_PATH "world/entities"

/// Get blob path from cookie.
///
/// Return NULL on failure.
static char *get_blob_path(uint32_t cookie)
{
  char *path;
  if(asprintf(&path, ENTITIES_PATH "/blobs/%08x", cookie) == -1)
    return NULL;

  return path;
}

/// Get active links path which is a constant.
///
/// Always succeed.
static const char *get_active_links_path(void)
{
  return ENTITIES_PATH "/active";
}

/// Get active link path from cookie.
///
/// Return NULL on failure.
static char *get_active_link_path(uint32_t cookie)
{
  char *path;
  if(asprintf(&path, ENTITIES_PATH "/active/%08x", cookie) == -1)
    return NULL;

  return path;
}

/// Get inactive path from cookie and entity position.
static char *get_inactive_link_path(ivec3_t chunk_position, uint32_t cookie)
{
  char *path;
  if(asprintf(&path, ENTITIES_PATH "/inactive/%d/%d/%d/%08x", chunk_position.x, chunk_position.y, chunk_position.z, cookie) == -1)
    return NULL;

  return path;
}

/// Get path to directory storing inactive links for chunk at specified position.
///
/// Return NULL on failure.
static char *get_inactive_links_path(ivec3_t chunk_position)
{
  char *path;
  if(asprintf(&path, ENTITIES_PATH "/inactive/%d/%d/%d", chunk_position.x, chunk_position.y, chunk_position.z) == -1)
    return NULL;

  return path;
}

/// Ensure directory containing path exist.
///
/// Return non-zero value on failure.
static int ensure_directory(const char *path)
{
  char *dirpath = parent(path);
  int result = mkdir_recursive(dirpath);
  free(dirpath);
  return result;
}

/// Write entity to serializer.
///
/// Return non-zero value on failure.
static int entity_write(libserde_serializer_t serializer, const struct voxy_entity *entity, struct voxy_entity_registry *entity_registry)
{
  libserde_serializer_try_write(serializer, entity->id, error);
  libserde_serializer_try_write(serializer, entity->position, error);
  libserde_serializer_try_write(serializer, entity->rotation, error);
  libserde_serializer_try_write(serializer, entity->velocity, error);
  libserde_serializer_try_write(serializer, entity->grounded, error);

  const struct voxy_entity_info info = voxy_entity_registry_query_entity(entity_registry, entity->id);
  if(info.serialize_opaque && info.serialize_opaque(serializer, entity->opaque) != 0)
    goto error;

  return 0;
error:
  return -1;
}

/// Write entity to path.
///
/// Return non-zero value on failure.
static int entity_write_to(const char *path, const struct voxy_entity *entity, struct voxy_entity_registry *entity_registry)
{
  int result = 0;

  if(ensure_directory(path) != 0)
  {
    result = -1;
    goto out1;
  }

  libserde_serializer_t serializer = libserde_serializer_create(path);
  if(!serializer)
  {
    result = -1;
    goto out1;
  }

  if(entity_write(serializer, entity, entity_registry) != 0)
  {
    result = -1;
    goto out2;
  }

out2:
  libserde_serializer_destroy(serializer);
out1:
  return result;
}

/// Write entity to path.
///
/// The serializer is created exclusively.
///
/// Return non-zero value on failure.
static int entity_write_to_exclusive(const char *path, int *exist, const struct voxy_entity *entity, struct voxy_entity_registry *entity_registry)
{
  int result = 0;

  if(ensure_directory(path) != 0)
  {
    result = -1;
    goto out1;
  }

  libserde_serializer_t serializer = libserde_serializer_create_exclusive(path, exist);
  if(!serializer)
  {
    result = -1;
    goto out1;
  }

  if(entity_write(serializer, entity, entity_registry) != 0)
  {
    result = -1;
    goto out2;
  }

out2:
  libserde_serializer_destroy(serializer);
out1:
  return result;
}

/// Read entity from serializer.
///
/// Return non-zero value on failure.
static int entity_read(libserde_deserializer_t deserializer, struct voxy_entity *entity, struct voxy_entity_registry *entity_registry)
{
  libserde_deserializer_try_read(deserializer, entity->id, error);
  libserde_deserializer_try_read(deserializer, entity->position, error);
  libserde_deserializer_try_read(deserializer, entity->rotation, error);
  libserde_deserializer_try_read(deserializer, entity->velocity, error);
  libserde_deserializer_try_read(deserializer, entity->grounded, error);

  const struct voxy_entity_info info = voxy_entity_registry_query_entity(entity_registry, entity->id);
  if(info.deserialize_opaque && !(entity->opaque = info.deserialize_opaque(deserializer)))
    goto error;
  else
    entity->opaque = NULL;

  return 0;
error:
  return -1;
}

/// Read entity from path.
///
/// Return non-zero value on failure.
static int entity_read_from(const char *path, struct voxy_entity *entity, struct voxy_entity_registry *entity_registry)
{
  int result = 0;

  libserde_deserializer_t deserializer = libserde_deserializer_create(path);
  if(!deserializer)
  {
    result = -1;
    goto out1;
  }

  if(entity_read(deserializer, entity, entity_registry) != 0)
  {
    result = -1;
    goto out2;
  }

out2:
  libserde_deserializer_destroy(deserializer);
out1:
  return result;
}

/// Create blob for the entity in the database.
///
/// This allocates a blob for the entity and serialize the entity data to the
/// blob. The allocated cookie is stored inside entity->cookie on successful
/// return. The path is also returned to facilatate creating links to the blob
/// in the filesystem.
///
/// The returned path need be be deallocated with free(3).
///
/// Return non-zero value on failure.
static int create_entity_blob(struct voxy_entity *entity, struct voxy_entity_registry *entity_registry, char **path)
{
  int result = 0;

retry:
  entity->cookie = rand();
  *path = get_blob_path(entity->cookie);
  if(!*path)
  {
    result = -1;
    goto out1;
  }

  int exist;
  if(entity_write_to_exclusive(*path, &exist, entity, entity_registry) != 0)
  {
    if(exist)
    {
      free(*path);
      goto retry;
    }

    result = -1;
    goto out2;
  }

out2:
  if(result != 0)
    free(*path);
out1:
  return result;
}

/// Create active link to entity blob.
///
/// This create the hard link <ENTITIES_PATH>/active/<COOKIE> to the entity
/// blob, where <COOKIE> is obtained from entity->cookie. The path argument is
/// the path to the entity blob as obtained from create_entity_blob().
///
/// Return non-zero value on failure.
static int entity_blob_create_active_link(const struct voxy_entity *entity, char *blob_path)
{
  int result = 0;

  char *link_path = get_active_link_path(entity->cookie);
  if(!link_path)
  {
    result = -1;
    goto out1;
  }

  if(ensure_directory(link_path) != 0)
  {
    result = -1;
    goto out2;
  }

  if(create_hard_link(link_path, blob_path) != 0)
  {
    result = -1;
    goto out2;
  }

out2:
  free(link_path);
out1:
  return result;
}

/// Parse u32 value from hex string.
///
/// The passed string is formatted as if with %08x specifier with printf.
///
/// Return non-zero value on failure.
static int u32_from_hex_str(const char *str, uint32_t *value)
{
  *value = 0;

  for(unsigned i=0; i<8; ++i)
  {
    const char c = str[i];
    const int digit = c >= '0' && c <= '9' ? c - '0'
                    : c >= 'a' && c <= 'f' ? c - 'a' + 10
                    : -1;

    if(digit == -1)
      return -1;

    *value <<= 4;
    *value |= digit;
  }

  if(str[8] != '\0')
    return -1;

  return 0;
}

/// Create an entity in the database.
///
/// This should be called when an entity is spawned. A blob will be allocated
/// for the entity and the entity will be put in loaded state i.e. there will be
/// a hard link to it under <ENTITIES_PATH>/active.
///
/// Return non-zero value on failure.
int voxy_entity_database_create_entity(struct voxy_entity *entity, struct voxy_entity_registry *entity_registry)
{
  int result = 0;

  char *blob_path;
  if(create_entity_blob(entity, entity_registry, &blob_path) != 0)
  {
    result = -1;
    goto out1;
  }

  if(entity_blob_create_active_link(entity, blob_path) != 0)
  {
    result = -1;
    goto out2;
  }

out2:
  free(blob_path);
out1:
  return result;
}

/// Destroy an entity in the database.
///
/// This should be called when an entity is despawned. This assumes that the
/// entity is in a loaded state i.e. there is a hard link to its blob under
/// <ENTITIES_PATH>/active. The entity blob will be deallocated and the hard
/// link to it will be removed.
///
/// A blob will be allocated
/// for the entity and the entity will be put in loaded state i.e. there will be
/// a hard link to it under <ENTITIES_PATH>/active.
///
/// Return non-zero value on failure.
int voxy_entity_database_destroy_entity(const struct voxy_entity *entity)
{
  int result = 0;

  char *blob_path = get_blob_path(entity->cookie);
  if(!blob_path)
  {
    result = -1;
    goto out1;
  }


  char *link_path = get_active_link_path(entity->cookie);
  if(!link_path)
  {
    result = -1;
    goto out2;
  }

  if(file_remove(blob_path) != 0)
  {
    result = -1;
    goto out3;
  }

  if(file_remove(link_path) != 0)
  {
    result = -1;
    goto out3;
  }

out3:
  free(link_path);
out2:
  free(blob_path);
out1:
  return result;
}

/// Update an entity in the database.
///
/// Return non-zero value on failure.
int voxy_entity_database_update_entity(const struct voxy_entity *entity, struct voxy_entity_registry *entity_registry)
{
  int result = 0;

  char *blob_path = get_blob_path(entity->cookie);
  if(!blob_path)
  {
    result = -1;
    goto out1;
  }

  if(entity_write_to(blob_path, entity, entity_registry) != 0)
  {
    result = -1;
    goto out2;
  }

out2:
  free(blob_path);
out1:
  return result;
}

/// Commit an entity in the database.
///
/// This should be called when an entity is to be unloaded. This move the hard
/// link to the entity under <ENTITIES_PATH>/active to
/// <ENTITIES_PATH>/<X>/<Y>/<Z> depending on which chunk the entity is in.
///
/// Return non-zero value on failure.
int voxy_entity_database_commit_entity(const struct voxy_entity *entity)
{
  int result = 0;

  char *active_link_path = get_active_link_path(entity->cookie);
  if(!active_link_path)
  {
    result = -1;
    goto out1;
  }

  char *inactive_link_path = get_inactive_link_path(get_chunk_position_f(entity->position), entity->cookie);
  if(!inactive_link_path)
  {
    result = -1;
    goto out2;
  }

  if(ensure_directory(inactive_link_path) != 0)
  {
    result = -1;
    goto out2;
  }

  if(file_rename(active_link_path, inactive_link_path) != 0)
  {
    result = -1;
    goto out3;
  }

out3:
  free(active_link_path);
out2:
  free(inactive_link_path);
out1:
  return result;
}

/// Commit an entity in the database.
///
/// This should be called after an entity is loaded. This move the hard link to
/// the entity under <ENTITIES_PATH>/<X>/<Y>/<Z> to <ENTITIES_PATH>/active.
///
/// Return non-zero value on failure.
int voxy_entity_database_uncommit_entity(const struct voxy_entity *entity, ivec3_t chunk_position)
{
  int result = 0;

  char *active_link_path = get_active_link_path(entity->cookie);
  if(!active_link_path)
  {
    result = -1;
    goto out1;
  }

  char *inactive_link_path = get_inactive_link_path(chunk_position, entity->cookie);
  if(!inactive_link_path)
  {
    result = -1;
    goto out2;
  }

  if(ensure_directory(active_link_path) != 0)
  {
    result = -1;
    goto out2;
  }

  if(file_rename(inactive_link_path, active_link_path) != 0)
  {
    result = -1;
    goto out3;
  }

out3:
  free(active_link_path);
out2:
  free(inactive_link_path);
out1:
  return result;
}

/// Load entities from given directory.
///
/// Return non-zero value on failure.
static int load_entities(const char *dirpath, struct voxy_entities *entities, struct voxy_entity_registry *entity_registry)
{
  int result = 0;

  DYNAMIC_ARRAY_INIT(*entities);

  DIR *dirp = opendir(dirpath);
  if(!dirp)
    goto out1;

  struct dirent *dirent;
  while((dirent = readdir(dirp)))
    if(dirent->d_type == DT_REG)
    {
      struct voxy_entity entity;

      char *filepath;
      asprintf(&filepath, "%s/%s", dirpath, dirent->d_name);
      if(!dirpath)
      {
        result = -1;
        goto out2;
      }

      if(u32_from_hex_str(dirent->d_name, &entity.cookie) != 0)
      {
        free(filepath);
        result = -1;
        goto out2;
      }

      if(entity_read_from(filepath, &entity, entity_registry) != 0)
      {
        free(filepath);
        result = -1;
        goto out2;
      }

      DYNAMIC_ARRAY_APPEND(*entities, entity);
      free(filepath);
    }

out2:
  closedir(dirp);
out1:
  if(result != 0)
    DYNAMIC_ARRAY_CLEAR(*entities);
  return result;
}

/// Load active entities.
///
/// Return non-zero value on failure.
int voxy_entity_database_load_active_entities(struct voxy_entities *entities, struct voxy_entity_registry *entity_registry)
{
  return load_entities(get_active_links_path(), entities, entity_registry);
}

/// Load inactive entities in given chunk position.
///
/// Return non-zero value on failure.
int voxy_entity_database_load_inactive_entities(ivec3_t chunk_position, struct voxy_entities *entities, struct voxy_entity_registry *entity_registry)
{
  int result = 0;

  char *links_path = get_inactive_links_path(chunk_position);
  if(!links_path)
  {
    result = -1;
    goto out1;
  }

  if(load_entities(links_path, entities, entity_registry) != 0)
  {
    result = -1;
    goto out2;
  }

out2:
  free(links_path);
out1:
  return result;
}

