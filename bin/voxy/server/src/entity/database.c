#include "database.h"

#include "chunk/coordinates.h"

#include <libserde/serializer.h>
#include <libserde/deserializer.h>

#include <libcommon/core/log.h>
#include <libcommon/core/fs.h>

#include <sqlite3.h>

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

#define VOXY_ENTITY_DATABASE_PATH "world/entities.db"

#define SQLITE_TRY_EXEC(rc, conn, errmsg, sql, label) \
  if(sqlite3_exec(conn, sql, NULL, NULL, &errmsg) != SQLITE_OK) \
  { \
    LOG_ERROR("Failed to exec sql statement: %s: %s", #sql, errmsg); \
    sqlite3_free(errmsg); \
    rc = -1; \
    goto out; \
  }

#define SQLITE_TRY_PREPARE_ONCE(rc, conn, stmt, sql, label) \
  static sqlite3_stmt *stmt; \
  if(!stmt && sqlite3_prepare_v2(conn, sql, -1, &stmt,  NULL) != SQLITE_OK) \
  { \
    LOG_ERROR("Failed to prepare sql statement: %s: %s", #sql, sqlite3_errmsg(conn)); \
    rc = -1; \
    goto out; \
  }

int voxy_entity_database_init(struct voxy_entity_database *database)
{
  int rc = 0;
  char *errmsg;

  if(sqlite3_open(VOXY_ENTITY_DATABASE_PATH, &database->conn) != SQLITE_OK)
  {
    LOG_ERROR("Failed to open entity database %s: %s", VOXY_ENTITY_DATABASE_PATH, sqlite3_errmsg(database->conn));
    rc = -1;
    goto out;
  }

  SQLITE_TRY_EXEC(rc, database->conn, errmsg, "CREATE TABLE IF NOT EXISTS entities(id INTEGER PRIMARY KEY, data BLOB NOT NULL) STRICT;", out);
  SQLITE_TRY_EXEC(rc, database->conn, errmsg, "CREATE TABLE IF NOT EXISTS active_entities(id INTEGER PRIMARY KEY, FOREIGN KEY(id) REFERENCES entities(id) ON DELETE CASCADE) STRICT;", out);
  SQLITE_TRY_EXEC(rc, database->conn, errmsg, "CREATE TABLE IF NOT EXISTS inactive_entities(id INTEGER PRIMARY KEY, x INTEGER NOT NULL, y INTEGER NOT NULL, z INTEGER NOT NULL, FOREIGN KEY(id) REFERENCES entities(id) ON DELETE CASCADE) STRICT;", out);
  SQLITE_TRY_EXEC(rc, database->conn, errmsg, "PRAGMA foreign_keys = ON;", out);

out:
  if(rc != SQLITE_OK)
    sqlite3_close(database->conn);
  return rc;
}

void voxy_entity_database_fini(struct voxy_entity_database *database)
{
  sqlite3_close(database->conn);
}

int voxy_entity_database_begin_transaction(struct voxy_entity_database *database)
{
  int rc = 0;

  SQLITE_TRY_PREPARE_ONCE(rc, database->conn, stmt, "BEGIN TRANSACTION", out);
  if(sqlite3_step(stmt) != SQLITE_DONE)
  {
    rc = -1;
    goto out_reset;
  }

out_reset:
  sqlite3_reset(stmt);
out:
  return rc;
}

int voxy_entity_database_end_transaction(struct voxy_entity_database *database)
{
  int rc = 0;

  SQLITE_TRY_PREPARE_ONCE(rc, database->conn, stmt, "END TRANSACTION", out);
  if(sqlite3_step(stmt) != SQLITE_DONE)
  {
    rc = -1;
    goto out_reset;
  }

out_reset:
  sqlite3_reset(stmt);
out:
  return rc;
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
  entity->opaque = NULL;

  libserde_deserializer_destroy(deserializer);
  return 0;

err_read:
  libserde_deserializer_destroy(deserializer);
err_create:
  return -1;
}

int voxy_entity_database_create(struct voxy_entity_database *database, struct voxy_entity_registry *entity_registry, struct voxy_entity *entity)
{
  int rc = 0;

  SQLITE_TRY_PREPARE_ONCE(rc, database->conn, stmt1, "INSERT INTO entities (data) VALUES (?);", out);
  SQLITE_TRY_PREPARE_ONCE(rc, database->conn, stmt2, "INSERT INTO active_entities (id) VALUES (?);", out);

  char *buf;
  size_t len;
  if((rc = entity_serialize(entity, &buf, &len) != 0))
  {
    rc = -1;
    goto out;
  }

  if(sqlite3_bind_blob(stmt1, 1, buf, len, SQLITE_STATIC) != SQLITE_OK) { LOG_ERROR("Failed to bind ql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_free_buf; }
  if(sqlite3_step(stmt1) != SQLITE_DONE) { LOG_ERROR("Failed to step sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset1; }

  if(sqlite3_bind_int(stmt2, 1, (entity->db_id = sqlite3_last_insert_rowid(database->conn))) != SQLITE_OK) { LOG_ERROR("Failed to bind sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset1; }
  if(sqlite3_step(stmt2) != SQLITE_DONE) { LOG_ERROR("Failed to step sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset2; }

out_reset2:
  sqlite3_reset(stmt2);
  sqlite3_clear_bindings(stmt2);
out_reset1:
  sqlite3_reset(stmt1);
  sqlite3_clear_bindings(stmt1);
out_free_buf:
  free(buf);
out:
  return rc;
}

int voxy_entity_database_destroy(struct voxy_entity_database *database, struct voxy_entity *entity)
{
  int rc = 0;

  SQLITE_TRY_PREPARE_ONCE(rc, database->conn, stmt, "DELETE FROM entities WHERE id = (?)", out);

  if(sqlite3_bind_int64(stmt, 1, entity->db_id) != SQLITE_OK)
  {
    LOG_ERROR("Failed to bind blob to sql statement: %s", sqlite3_errmsg(database->conn));
    rc = -1;
    goto out;
  }

  if(sqlite3_step(stmt) != SQLITE_DONE)
  {
    LOG_ERROR("Failed to step sql statement: %s", sqlite3_errmsg(database->conn));
    rc = -1;
    goto out_reset;
  }

out_reset:
  sqlite3_reset(stmt);
  sqlite3_clear_bindings(stmt);
out:
  return rc;
}

int voxy_entity_database_save(struct voxy_entity_database *database, struct voxy_entity_registry *entity_registry, const struct voxy_entity *entity)
{
  int rc = 0;

  SQLITE_TRY_PREPARE_ONCE(rc, database->conn, stmt, "UPDATE entities SET data = (?) WHERE id = (?)", out);

  char *buf;
  size_t len;
  if((rc = entity_serialize(entity, &buf, &len) != 0))
  {
    LOG_ERROR("Failed to serialize entity");
    rc = -1;
    goto out;
  }

  if(sqlite3_bind_blob(stmt, 1, buf, len, SQLITE_STATIC) != SQLITE_OK)
  {
    LOG_ERROR("Failed to bind blob to sql statement: %s", sqlite3_errmsg(database->conn));
    rc = -1;
    goto out_free_buf;
  }

  if(sqlite3_bind_int64(stmt, 2, entity->db_id) != SQLITE_OK)
  {
    LOG_ERROR("Failed to bind blob to sql statement: %s", sqlite3_errmsg(database->conn));
    rc = -1;
    goto out_reset;
  }

  if(sqlite3_step(stmt) != SQLITE_DONE)
  {
    LOG_ERROR("Failed to step sql statement: %s", sqlite3_errmsg(database->conn));
    rc = -1;
    goto out_reset;
  }

out_reset:
  sqlite3_reset(stmt);
  sqlite3_clear_bindings(stmt);
out_free_buf:
  free(buf);
out:
  return rc;
}

int voxy_entity_database_load(struct voxy_entity_database *database, struct voxy_entity_registry *entity_registry, struct voxy_entity *entity)
{
  int rc = 0;

  SQLITE_TRY_PREPARE_ONCE(rc, database->conn, stmt, "SELECT data FROM entities WHERE id = (?);", out);

  if(sqlite3_bind_int64(stmt, 1, entity->db_id) != SQLITE_OK) { LOG_ERROR("Failed to bind sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out; }
  if(sqlite3_step(stmt) != SQLITE_ROW) { LOG_ERROR("Failed to step sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset; }
  if(entity_deserialize(entity, sqlite3_column_blob(stmt, 0), sqlite3_column_bytes(stmt, 0)) != 0)
  {
    LOG_ERROR("Failed to deserialize entity");
    rc = -1;
    goto out_reset;
  }
  if(sqlite3_step(stmt) != SQLITE_DONE) { LOG_ERROR("Failed to step sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset; }

out_reset:
  sqlite3_reset(stmt);
  sqlite3_clear_bindings(stmt);
out:
  return rc;
}

int voxy_entity_database_uncommit(struct voxy_entity_database *database, int64_t db_id)
{
  int rc = 0;

  SQLITE_TRY_PREPARE_ONCE(rc, database->conn, stmt1, "INSERT INTO active_entities(id) VALUES (?)", out);
  SQLITE_TRY_PREPARE_ONCE(rc, database->conn, stmt2, "DELETE FROM inactive_entities WHERE id = (?)", out);

  if(sqlite3_bind_int64(stmt1, 1, db_id) != SQLITE_OK)
  {
    LOG_ERROR("Failed to bind blob to sql statement: %s", sqlite3_errmsg(database->conn));
    rc = -1;
    goto out;
  }

  if(sqlite3_step(stmt1) != SQLITE_DONE)
  {
    LOG_ERROR("Failed to step sql statement: %s", sqlite3_errmsg(database->conn));
    rc = -1;
    goto out_reset1;
  }

  if(sqlite3_bind_int64(stmt2, 1, db_id) != SQLITE_OK)
  {
    LOG_ERROR("Failed to bind blob to sql statement: %s", sqlite3_errmsg(database->conn));
    rc = -1;
    goto out_reset1;
  }

  if(sqlite3_step(stmt2) != SQLITE_DONE)
  {
    LOG_ERROR("Failed to step sql statement: %s", sqlite3_errmsg(database->conn));
    rc = -1;
    goto out_reset2;
  }

out_reset2:
  sqlite3_reset(stmt2);
  sqlite3_clear_bindings(stmt2);
out_reset1:
  sqlite3_reset(stmt1);
  sqlite3_clear_bindings(stmt1);
out:
  return rc;
}

int voxy_entity_database_commit(struct voxy_entity_database *database, int64_t db_id, ivec3_t chunk_position)
{
  int rc = 0;

  SQLITE_TRY_PREPARE_ONCE(rc, database->conn, stmt1, "INSERT INTO inactive_entities(id, x, y, z) VALUES (?, ?, ?, ?)", out);
  SQLITE_TRY_PREPARE_ONCE(rc, database->conn, stmt2, "DELETE FROM active_entities WHERE id = (?)", out);

  if(sqlite3_bind_int64(stmt1, 1, db_id) != SQLITE_OK) { LOG_ERROR("Failed to bind blob to sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out; }
  if(sqlite3_bind_int(stmt1, 2, chunk_position.x) != SQLITE_OK) { LOG_ERROR("Failed to bind sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset1; }
  if(sqlite3_bind_int(stmt1, 3, chunk_position.y) != SQLITE_OK) { LOG_ERROR("Failed to bind sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset1; }
  if(sqlite3_bind_int(stmt1, 4, chunk_position.z) != SQLITE_OK) { LOG_ERROR("Failed to bind sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset1; }
  if(sqlite3_step(stmt1) != SQLITE_DONE) { LOG_ERROR("Failed to step sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset1; }

  if(sqlite3_bind_int64(stmt2, 1, db_id) != SQLITE_OK) { LOG_ERROR("Failed to bind blob to sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset1; }
  if(sqlite3_step(stmt2) != SQLITE_DONE) { LOG_ERROR("Failed to step sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset2; }

out_reset2:
  sqlite3_clear_bindings(stmt2);
  sqlite3_reset(stmt2);
out_reset1:
  sqlite3_clear_bindings(stmt1);
  sqlite3_reset(stmt1);
out:
  return rc;
}

int voxy_entity_database_load_active(struct voxy_entity_database *database, struct db_ids *db_ids)
{
  int rc = 0;
  int result;

  SQLITE_TRY_PREPARE_ONCE(rc, database->conn, stmt_select, "SELECT id FROM active_entities;", out);
  SQLITE_TRY_PREPARE_ONCE(rc, database->conn, stmt_delete, "DELETE FROM active_entities;", out);

  while((result = sqlite3_step(stmt_select)) == SQLITE_ROW)
  {
    int64_t db_id = sqlite3_column_int64(stmt_select, 0);
    DYNAMIC_ARRAY_APPEND(*db_ids, db_id);
  }

  if(result != SQLITE_DONE)
    goto out_reset_select;

  if(sqlite3_step(stmt_delete) != SQLITE_DONE)
    goto out_reset_delete;

out_reset_delete:
  sqlite3_reset(stmt_delete);
out_reset_select:
  sqlite3_reset(stmt_select);
out:
  return rc;
}

int voxy_entity_database_load_inactive(struct voxy_entity_database *database, ivec3_t chunk_position, struct db_ids *db_ids)
{
  int rc = 0;
  int result;

  SQLITE_TRY_PREPARE_ONCE(rc, database->conn, stmt_select, "SELECT id FROM inactive_entities WHERE x = (?) AND y = (?) AND z = (?);", out);
  SQLITE_TRY_PREPARE_ONCE(rc, database->conn, stmt_delete, "DELETE FROM inactive_entities WHERE x = (?) AND y = (?) AND z = (?);", out);

  if(sqlite3_bind_int(stmt_select, 1, chunk_position.x) != SQLITE_OK) { LOG_ERROR("Failed to bind sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out; }
  if(sqlite3_bind_int(stmt_select, 2, chunk_position.y) != SQLITE_OK) { LOG_ERROR("Failed to bind sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset_select; }
  if(sqlite3_bind_int(stmt_select, 3, chunk_position.z) != SQLITE_OK) { LOG_ERROR("Failed to bind sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset_select; }
  while((result = sqlite3_step(stmt_select)) == SQLITE_ROW)
  {
    int64_t db_id = sqlite3_column_int64(stmt_select, 0);
    DYNAMIC_ARRAY_APPEND(*db_ids, db_id);
  }
  if(result != SQLITE_DONE)
    goto out_reset_select;

  if(sqlite3_bind_int(stmt_delete, 1, chunk_position.x) != SQLITE_OK) { LOG_ERROR("Failed to bind sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset_select; }
  if(sqlite3_bind_int(stmt_delete, 2, chunk_position.y) != SQLITE_OK) { LOG_ERROR("Failed to bind sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset_delete; }
  if(sqlite3_bind_int(stmt_delete, 3, chunk_position.z) != SQLITE_OK) { LOG_ERROR("Failed to bind sql statement: %s", sqlite3_errmsg(database->conn)); rc = -1; goto out_reset_delete; }
  if(sqlite3_step(stmt_delete) != SQLITE_DONE)
    goto out_reset_delete;

out_reset_delete:
  sqlite3_reset(stmt_delete);
  sqlite3_clear_bindings(stmt_delete);
out_reset_select:
  sqlite3_reset(stmt_select);
  sqlite3_clear_bindings(stmt_select);
out:
  return rc;
}
