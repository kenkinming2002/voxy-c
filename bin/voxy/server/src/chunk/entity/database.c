#include "database.h"

#include "chunk/coordinates.h"
#include "sqlite3_utils.h"

#include <voxy/server/registry/entity.h>

#include <libserde/serializer.h>
#include <libserde/deserializer.h>

#include <libcore/profile.h>
#include <libcore/log.h>
#include <libcore/fs.h>
#include <libcore/format.h>

#include <sqlite3.h>
#include <stb_ds.h>

#include <stdlib.h>
#include <stdio.h>

static sqlite3 *conn;

static void cleanup(void)
{
  sqlite3_close(conn);
}

void voxy_entity_database_init(const char *world_directory)
{
  const char *directory = tformat("%s/entities", world_directory);
  if(mkdir_recursive(directory) != 0)
  {
    LOG_ERROR("Failed to create directory: %s", directory);
    exit(EXIT_FAILURE);
  }

  const char *path = tformat("%s/entities/data.db", world_directory);
  if(sqlite3_open(path, &conn) != SQLITE_OK)
  {
    LOG_ERROR("Failed to open entity database %s: %s", path, sqlite3_errmsg(conn));
    exit(EXIT_FAILURE);
  }

  atexit(cleanup);

  if(sqlite3_utils_exec(conn, "PRAGMA journal_mode = WAL;") != 0)
    exit(EXIT_FAILURE);

  if(sqlite3_utils_exec(conn, "PRAGMA foreign_keys = ON;") != 0)
    exit(EXIT_FAILURE);

  if(sqlite3_utils_exec(conn, "CREATE TABLE IF NOT EXISTS entities(id INTEGER PRIMARY KEY, data BLOB NOT NULL) STRICT;") != 0)
    exit(EXIT_FAILURE);

  if(sqlite3_utils_exec(conn, "CREATE TABLE IF NOT EXISTS active_entities(id INTEGER PRIMARY KEY, FOREIGN KEY(id) REFERENCES entities(id) ON DELETE CASCADE) STRICT;") != 0)
    exit(EXIT_FAILURE);

  if(sqlite3_utils_exec(conn, "CREATE TABLE IF NOT EXISTS inactive_entities(id INTEGER PRIMARY KEY, x INTEGER NOT NULL, y INTEGER NOT NULL, z INTEGER NOT NULL, FOREIGN KEY(id) REFERENCES entities(id) ON DELETE CASCADE) STRICT;") != 0)
    exit(EXIT_FAILURE);
}

void voxy_entity_database_begin_transaction(void)
{
  profile_scope;

  static sqlite3_stmt *stmt = NULL;
  if(sqlite3_utils_prepare_once(conn, &stmt, "BEGIN TRANSACTION;") != 0) exit(EXIT_FAILURE);
  if(sqlite3_utils_run(conn, stmt, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) exit(EXIT_FAILURE);
}

void voxy_entity_database_end_transaction(void)
{
  profile_scope;

  static sqlite3_stmt *stmt = NULL;
  if(sqlite3_utils_prepare_once(conn, &stmt, "END TRANSACTION;") != 0) exit(EXIT_FAILURE);
  if(sqlite3_utils_run(conn, stmt, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) exit(EXIT_FAILURE);
}

void voxy_entity_database_create(struct voxy_entity *entity)
{
  struct sqlite3_utils_blob blob;
  if(voxy_entity_serialize(entity, &blob) != 0)
    exit(EXIT_FAILURE);

  static sqlite3_stmt *stmt_insert_entities = NULL;
  if(sqlite3_utils_prepare_once(conn, &stmt_insert_entities, "INSERT INTO entities (data) VALUES (?);") != 0) exit(EXIT_FAILURE);
  if(sqlite3_utils_run(conn, stmt_insert_entities, SQLITE3_UTILS_TYPE_BLOB, blob, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) exit(EXIT_FAILURE);

  static sqlite3_stmt *stmt_insert_active_entities = NULL;
  if(sqlite3_utils_prepare_once(conn, &stmt_insert_active_entities, "INSERT INTO active_entities (id) VALUES (?);") != 0) exit(EXIT_FAILURE);
  if(sqlite3_utils_run(conn, stmt_insert_active_entities, SQLITE3_UTILS_TYPE_INT64, (entity->db_id = sqlite3_last_insert_rowid(conn)), SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) exit(EXIT_FAILURE);

  free(blob.data);
}

void voxy_entity_database_update(const struct voxy_entity *entity)
{
  struct sqlite3_utils_blob blob;
  if(voxy_entity_serialize(entity, &blob) != 0)
    exit(EXIT_FAILURE);

  static sqlite3_stmt *stmt_insert_entities = NULL;
  if(sqlite3_utils_prepare_once(conn, &stmt_insert_entities, "UPDATE entities SET data = (?) WHERE id = (?);") != 0) exit(EXIT_FAILURE);
  if(sqlite3_utils_run(conn, stmt_insert_entities, SQLITE3_UTILS_TYPE_BLOB, blob, SQLITE3_UTILS_TYPE_INT64, entity->db_id, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) exit(EXIT_FAILURE);

  free(blob.data);
}

void voxy_entity_database_destroy(struct voxy_entity *entity)
{
  static sqlite3_stmt *stmt_delete_entities = NULL;
  if(sqlite3_utils_prepare_once(conn, &stmt_delete_entities, "DELETE FROM entities WHERE id = (?);") != 0) exit(EXIT_FAILURE);
  if(sqlite3_utils_run(conn, stmt_delete_entities, SQLITE3_UTILS_TYPE_INT64, entity->db_id, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) exit(EXIT_FAILURE);
}

void voxy_entity_database_unload(struct voxy_entity *entity)
{
  const ivec3_t chunk_position = get_chunk_position_f(entity->position);

  static sqlite3_stmt *stmt_insert = NULL;
  if(sqlite3_utils_prepare_once(conn, &stmt_insert, "INSERT INTO inactive_entities(id, x, y, z) VALUES (?, ?, ?, ?)") != 0) exit(EXIT_FAILURE);
  if(sqlite3_utils_run(conn, stmt_insert, SQLITE3_UTILS_TYPE_INT64, entity->db_id, SQLITE3_UTILS_TYPE_INT, chunk_position.x, SQLITE3_UTILS_TYPE_INT, chunk_position.y, SQLITE3_UTILS_TYPE_INT, chunk_position.z, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) exit(EXIT_FAILURE);

  static sqlite3_stmt *stmt_delete = NULL;
  if(sqlite3_utils_prepare_once(conn, &stmt_delete, "DELETE FROM active_entities WHERE id = (?)") != 0) exit(EXIT_FAILURE);
  if(sqlite3_utils_run(conn, stmt_delete, SQLITE3_UTILS_TYPE_INT64, entity->db_id, SQLITE3_UTILS_TYPE_NONE, SQLITE3_UTILS_RETURN_TYPE_NONE) != 0) exit(EXIT_FAILURE);
}

static void free_ids_and_blobs(int64_t *ids, struct sqlite3_utils_blob *blobs)
{
  for(size_t i=0; i<arrlenu(blobs); ++i)
    free(blobs[i].data);

  arrfree(ids);
  arrfree(blobs);
}

static void voxy_entity_database_load_impl(struct voxy_entity **entities, int64_t *ids, struct sqlite3_utils_blob *blobs)
{
  *entities = NULL;
  for(size_t i=0; i<arrlenu(ids); ++i)
  {
    struct voxy_entity entity = {0};
    entity.alive = true;
    entity.db_id = ids[i];
    if(voxy_entity_deserialize(&entity, &blobs[i]) != 0)
      exit(EXIT_FAILURE);

    arrput(*entities, entity);
  }

  free_ids_and_blobs(ids, blobs);
}

void voxy_entity_database_load_active(struct voxy_entity **entities)
{
  static sqlite3_stmt *stmt = NULL;
  if(sqlite3_utils_prepare_once(conn, &stmt, "SELECT id, data FROM entities WHERE id IN (SELECT id FROM active_entities);") != 0)
    exit(EXIT_FAILURE);

  int64_t *ids;
  struct sqlite3_utils_blob *blobs;
  if(sqlite3_utils_run(conn, stmt,
    SQLITE3_UTILS_TYPE_NONE,
    SQLITE3_UTILS_RETURN_TYPE_ARRAY_INT64, &ids,
    SQLITE3_UTILS_RETURN_TYPE_ARRAY_BLOB, &blobs,
    SQLITE3_UTILS_RETURN_TYPE_NONE) != 0)
    exit(EXIT_FAILURE);

  voxy_entity_database_load_impl(entities, ids, blobs);
}

void voxy_entity_database_load_inactive(ivec3_t chunk_position, struct voxy_entity **entities)
{
  static sqlite3_stmt *stmt = NULL;
  if(sqlite3_utils_prepare_once(conn, &stmt, "SELECT id, data FROM entities WHERE id IN (SELECT id FROM inactive_entities WHERE x = (?) AND y = (?) AND z = (?));") != 0)
    exit(EXIT_FAILURE);

  int64_t *ids;
  struct sqlite3_utils_blob *blobs;
  if(sqlite3_utils_run(conn, stmt,
        SQLITE3_UTILS_TYPE_INT, chunk_position.x,
        SQLITE3_UTILS_TYPE_INT, chunk_position.y,
        SQLITE3_UTILS_TYPE_INT, chunk_position.z,
        SQLITE3_UTILS_TYPE_NONE,
        SQLITE3_UTILS_RETURN_TYPE_ARRAY_INT64, &ids,
        SQLITE3_UTILS_RETURN_TYPE_ARRAY_BLOB, &blobs,
        SQLITE3_UTILS_RETURN_TYPE_NONE) != 0)
    exit(EXIT_FAILURE);

  static sqlite3_stmt *stmt_insert = NULL;
  if(sqlite3_utils_prepare_once(conn, &stmt_insert, "INSERT INTO active_entities SELECT id FROM inactive_entities WHERE x = (?) AND y = (?) AND z = (?);") != 0)
    exit(EXIT_FAILURE);

  if(sqlite3_utils_run(conn, stmt_insert,
        SQLITE3_UTILS_TYPE_INT, chunk_position.x,
        SQLITE3_UTILS_TYPE_INT, chunk_position.y,
        SQLITE3_UTILS_TYPE_INT, chunk_position.z,
        SQLITE3_UTILS_TYPE_NONE,
        SQLITE3_UTILS_RETURN_TYPE_NONE) != 0)
    exit(EXIT_FAILURE);

  static sqlite3_stmt *stmt_delete = NULL;
  if(sqlite3_utils_prepare_once(conn, &stmt_delete, "DELETE FROM inactive_entities WHERE x = (?) AND y = (?) AND z = (?);") != 0)
    exit(EXIT_FAILURE);

  if(sqlite3_utils_run(conn, stmt_delete,
        SQLITE3_UTILS_TYPE_INT, chunk_position.x,
        SQLITE3_UTILS_TYPE_INT, chunk_position.y,
        SQLITE3_UTILS_TYPE_INT, chunk_position.z,
        SQLITE3_UTILS_TYPE_NONE,
        SQLITE3_UTILS_RETURN_TYPE_NONE) != 0)
    exit(EXIT_FAILURE);

  voxy_entity_database_load_impl(entities, ids, blobs);
}

