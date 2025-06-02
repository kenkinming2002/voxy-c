#include "database.h"

#include <voxy/server/registry/entity.h>

#include "sqlite3_utils.h"

#include <libserde/serializer.h>
#include <libserde/deserializer.h>

#include <libcore/profile.h>
#include <libcore/log.h>
#include <libcore/fs.h>
#include <libcore/format.h>

#include <sqlite3.h>

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

int voxy_entity_database_init(struct voxy_entity_database *database, const char *world_directory)
{
  int rc = 0;

  const char *directory = tformat("%s/entities", world_directory);
  if(mkdir_recursive(directory) != 0)
  {
    LOG_ERROR("Failed to create directory: %s", directory);
    rc = -1;
    goto out;
  }

  const char *path = tformat("%s/entities/data.db", world_directory);
  if(sqlite3_open(path, &database->conn) != SQLITE_OK)
  {
    LOG_ERROR("Failed to open entity database %s: %s", path, sqlite3_errmsg(database->conn));
    rc = -1;
    goto out;
  }

  if(sqlite3_utils_exec(database->conn, "PRAGMA journal_mode = WAL;") != 0) { rc = -1; goto out; }
  if(sqlite3_utils_exec(database->conn, "PRAGMA foreign_keys = ON;") != 0) { rc = -1; goto out; }

  if(sqlite3_utils_exec(database->conn, "CREATE TABLE IF NOT EXISTS entities(id INTEGER PRIMARY KEY, data BLOB NOT NULL) STRICT;") != 0) { rc = -1; goto out; }
  if(sqlite3_utils_exec(database->conn, "CREATE TABLE IF NOT EXISTS active_entities(id INTEGER PRIMARY KEY, FOREIGN KEY(id) REFERENCES entities(id) ON DELETE CASCADE) STRICT;") != 0) { rc = -1; goto out; }
  if(sqlite3_utils_exec(database->conn, "CREATE TABLE IF NOT EXISTS inactive_entities(id INTEGER PRIMARY KEY, x INTEGER NOT NULL, y INTEGER NOT NULL, z INTEGER NOT NULL, FOREIGN KEY(id) REFERENCES entities(id) ON DELETE CASCADE) STRICT;") != 0) { rc = -1; goto out; }

out:
  if(rc != 0)
    sqlite3_close(database->conn);
  return rc;
}

void voxy_entity_database_fini(struct voxy_entity_database *database)
{
  sqlite3_close(database->conn);
}

int voxy_entity_database_begin_transaction(struct voxy_entity_database *database)
{
  profile_scope;

  static sqlite3_stmt *stmt = NULL;
  if(sqlite3_utils_prepare_once(database->conn, &stmt, "BEGIN TRANSACTION;") != 0) return -1;
  if(sqlite3_utils_run(database->conn, stmt, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) return -1;

  return 0;
}

int voxy_entity_database_end_transaction(struct voxy_entity_database *database)
{
  profile_scope;

  static sqlite3_stmt *stmt = NULL;
  if(sqlite3_utils_prepare_once(database->conn, &stmt, "END TRANSACTION;") != 0) return -1;
  if(sqlite3_utils_run(database->conn, stmt, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) return -1;

  return 0;
}

static int entity_serialize(const struct voxy_entity *entity, char **buf, size_t *len)
{
  libserde_serializer_t serializer = libserde_serializer_create_mem(buf, len);
  if(!serializer)
    goto err_create;

  libserde_serializer_try_write(serializer, entity->id, err_write);
  libserde_serializer_try_write(serializer, entity->position, err_write);
  libserde_serializer_try_write(serializer, entity->rotation, err_write);
  libserde_serializer_try_write(serializer, entity->velocity, err_write);
  libserde_serializer_try_write(serializer, entity->grounded, err_write);

  const struct voxy_entity_info info = voxy_query_entity(entity->id);
  if(info.serialize_opaque(serializer, entity->opaque) != 0)
    goto err_write;

  libserde_serializer_destroy(serializer);
  return 0;

err_write:
  libserde_serializer_destroy(serializer);
  free(buf);
err_create:
  return -1;
}

static int entity_deserialize(struct voxy_entity *entity, const char *buf, size_t len)
{
  libserde_deserializer_t deserializer = libserde_deserializer_create_mem(buf, len);
  if(!deserializer)
    goto err_create;

  libserde_deserializer_try_read(deserializer, entity->id, err_read);
  libserde_deserializer_try_read(deserializer, entity->position, err_read);
  libserde_deserializer_try_read(deserializer, entity->rotation, err_read);
  libserde_deserializer_try_read(deserializer, entity->velocity, err_read);
  libserde_deserializer_try_read(deserializer, entity->grounded, err_read);

  const struct voxy_entity_info info = voxy_query_entity(entity->id);
  if(info.deserialize_opaque(deserializer, &entity->opaque) != 0)
    goto err_read;

  libserde_deserializer_destroy(deserializer);
  return 0;

err_read:
  libserde_deserializer_destroy(deserializer);
err_create:
  return -1;
}

int voxy_entity_database_create(struct voxy_entity_database *database, struct voxy_entity *entity)
{
  int rc = 0;

  char *buf;
  size_t len;
  if((rc = entity_serialize(entity, &buf, &len) != 0))
  {
    rc = -1;
    goto out;
  }

  static sqlite3_stmt *stmt_insert_entities = NULL;
  if(sqlite3_utils_prepare_once(database->conn, &stmt_insert_entities, "INSERT INTO entities (data) VALUES (?);") != 0) { rc = -1; goto out_free_buf; }
  if(sqlite3_utils_run(database->conn, stmt_insert_entities, SQLITE3_UTILS_TYPE_BLOB, buf, len, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) { rc = -1; goto out_free_buf; }

  static sqlite3_stmt *stmt_insert_active_entities = NULL;
  if(sqlite3_utils_prepare_once(database->conn, &stmt_insert_active_entities, "INSERT INTO active_entities (id) VALUES (?);") != 0) { rc = -1; goto out_free_buf; }
  if(sqlite3_utils_run(database->conn, stmt_insert_active_entities, SQLITE3_UTILS_TYPE_INT64, (entity->db_id = sqlite3_last_insert_rowid(database->conn)), SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) { rc = -1; goto out_free_buf; }

out_free_buf:
  free(buf);
out:
  return rc;
}

int voxy_entity_database_destroy(struct voxy_entity_database *database, struct voxy_entity *entity)
{
  int rc = 0;

  static sqlite3_stmt *stmt = NULL;
  if(sqlite3_utils_prepare_once(database->conn, &stmt, "DELETE FROM entities WHERE id = (?);") != 0) { rc = -1; goto out; }
  if(sqlite3_utils_run(database->conn, stmt, SQLITE3_UTILS_TYPE_INT64, entity->db_id, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) { rc = -1; goto out; }

out:
  return rc;
}

int voxy_entity_database_save(struct voxy_entity_database *database, const struct voxy_entity *entity)
{
  int rc = 0;
  char *buf;
  size_t len;

  if((rc = entity_serialize(entity, &buf, &len) != 0))
  {
    LOG_ERROR("Failed to serialize entity");
    rc = -1;
    goto out;
  }

  static sqlite3_stmt *stmt = NULL;
  if(sqlite3_utils_prepare_once(database->conn, &stmt, "UPDATE entities SET data = (?) WHERE id = (?);") != 0) { rc = -1; goto out_free_buf; }
  if(sqlite3_utils_run(database->conn, stmt, SQLITE3_UTILS_TYPE_BLOB, (struct sqlite3_utils_blob){ .data = buf, .length = len, }, SQLITE3_UTILS_TYPE_INT64, entity->db_id, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) { rc = -1; goto out_free_buf; }

out_free_buf:
  free(buf);
out:
  return rc;
}

int voxy_entity_database_load(struct voxy_entity_database *database, struct voxy_entity *entity)
{
  int rc = 0;
  struct sqlite3_utils_blob blob;

  static sqlite3_stmt *stmt = NULL;
  if(sqlite3_utils_prepare_once(database->conn, &stmt, "SELECT data FROM entities WHERE id = (?);") != 0) { rc = -1; goto out; }
  if(sqlite3_utils_run(database->conn, stmt, SQLITE3_UTILS_TYPE_INT64, entity->db_id, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_BLOB, &blob, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) { rc = -1; goto out; }

  if(entity_deserialize(entity, blob.data, blob.length) != 0)
  {
    LOG_ERROR("Failed to deserialize entity");
    rc = -1;
    goto out_free_buf;
  }

out_free_buf:
  free(blob.data);
out:
  return rc;
}

int voxy_entity_database_uncommit(struct voxy_entity_database *database, int64_t db_id)
{
  int rc = 0;

  static sqlite3_stmt *stmt_insert = NULL;
  if(sqlite3_utils_prepare_once(database->conn, &stmt_insert, "INSERT INTO active_entities(id) VALUES (?)") != 0) { rc = -1; goto out; }
  if(sqlite3_utils_run(database->conn, stmt_insert, SQLITE3_UTILS_TYPE_INT64, db_id, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) { rc = -1; goto out; }

  static sqlite3_stmt *stmt_delete = NULL;
  if(sqlite3_utils_prepare_once(database->conn, &stmt_delete, "DELETE FROM inactive_entities WHERE id = (?)") != 0) { rc = -1; goto out; }
  if(sqlite3_utils_run(database->conn, stmt_delete, SQLITE3_UTILS_TYPE_INT64, db_id, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) { rc = -1; goto out; }

out:
  return rc;
}

int voxy_entity_database_commit(struct voxy_entity_database *database, int64_t db_id, ivec3_t chunk_position)
{
  int rc = 0;

  static sqlite3_stmt *stmt_insert = NULL;
  if(sqlite3_utils_prepare_once(database->conn, &stmt_insert, "INSERT INTO inactive_entities(id, x, y, z) VALUES (?, ?, ?, ?)") != 0) { rc = -1; goto out; }
  if(sqlite3_utils_run(database->conn, stmt_insert, SQLITE3_UTILS_TYPE_INT64, db_id, SQLITE3_UTILS_TYPE_INT, chunk_position.x, SQLITE3_UTILS_TYPE_INT, chunk_position.y, SQLITE3_UTILS_TYPE_INT, chunk_position.z, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) { rc = -1; goto out; }

  static sqlite3_stmt *stmt_delete = NULL;
  if(sqlite3_utils_prepare_once(database->conn, &stmt_delete, "DELETE FROM active_entities WHERE id = (?)") != 0) { rc = -1; goto out; }
  if(sqlite3_utils_run(database->conn, stmt_delete, SQLITE3_UTILS_TYPE_INT64, db_id, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) { rc = -1; goto out; }

out:
  return rc;
}

int voxy_entity_database_load_active(struct voxy_entity_database *database, int64_t **db_ids)
{
  int rc = 0;

  static sqlite3_stmt *stmt_insert = NULL;
  if(sqlite3_utils_prepare_once(database->conn, &stmt_insert, "SELECT id FROM active_entities;") != 0) { rc = -1; goto out; }
  if(sqlite3_utils_run(database->conn, stmt_insert, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_ARRAY_INT64, db_ids, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) { rc = -1; goto out; }

  static sqlite3_stmt *stmt_delete = NULL;
  if(sqlite3_utils_prepare_once(database->conn, &stmt_delete, "DELETE FROM active_entities WHERE id = (?)") != 0) { rc = -1; goto out; }
  if(sqlite3_utils_run(database->conn, stmt_delete, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) { rc = -1; goto out; }

out:
  return rc;
}

int voxy_entity_database_load_inactive(struct voxy_entity_database *database, ivec3_t chunk_position, int64_t **db_ids)
{
  int rc = 0;

  static sqlite3_stmt *stmt_insert = NULL;
  if(sqlite3_utils_prepare_once(database->conn, &stmt_insert, "SELECT id FROM inactive_entities WHERE x = (?) AND y = (?) AND z = (?);") != 0) { rc = -1; goto out; }
  if(sqlite3_utils_run(database->conn, stmt_insert, SQLITE3_UTILS_TYPE_INT, chunk_position.x, SQLITE3_UTILS_TYPE_INT, chunk_position.y, SQLITE3_UTILS_TYPE_INT, chunk_position.z, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_ARRAY_INT64, db_ids, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) { rc = -1; goto out; }

  static sqlite3_stmt *stmt_delete = NULL;
  if(sqlite3_utils_prepare_once(database->conn, &stmt_delete, "DELETE FROM inactive_entities WHERE x = (?) AND y = (?) AND z = (?);") != 0) { rc = -1; goto out; }
  if(sqlite3_utils_run(database->conn, stmt_delete, SQLITE3_UTILS_TYPE_INT, chunk_position.x, SQLITE3_UTILS_TYPE_INT, chunk_position.y, SQLITE3_UTILS_TYPE_INT, chunk_position.z, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) { rc = -1; goto out; }

out:
  return rc;
}
