#include "sqlite3_utils.h"

#include <libcore/log.h>

#include <stb_ds.h>

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static void sqlite3_utils_report_error(sqlite3 *db)
{
  LOG_ERROR("sqlite3: %s", sqlite3_errmsg(db));
}

static void *memdup(const void *buf, size_t len)
{
  void *res = malloc(len);
  memcpy(res, buf, len);
  return res;
}

int sqlite3_utils_prepare_once(sqlite3 *db, sqlite3_stmt **stmt, const char *sql)
{
  if(*stmt)
    return 0;

  if(sqlite3_prepare_v2(db, sql, -1, stmt, NULL) != SQLITE_OK)
  {
    sqlite3_utils_report_error(db);
    return -1;
  }

  return 0;
}

int sqlite3_utils_run(sqlite3 *db, sqlite3_stmt *stmt, ...)
{
  int rc = 0;
  int result;

  va_list ap;
  va_start(ap, stmt);
  for(int index = 1;; ++index)
  {
    switch(va_arg(ap, enum sqlite3_utils_type))
    {
    case SQLITE3_UTILS_TYPE_INT:
      {
        const int value = va_arg(ap, int);
        result = sqlite3_bind_int(stmt, index, value) != SQLITE_OK;
        break;
      }
    case SQLITE3_UTILS_TYPE_INT64:
      {
        const int64_t value = va_arg(ap, int64_t);
        result = sqlite3_bind_int64(stmt, index, value) != SQLITE_OK;
        break;
      }
    case SQLITE3_UTILS_TYPE_DOUBLE:
      {
        const double value = va_arg(ap, double);
        result = sqlite3_bind_double(stmt, index, value) != SQLITE_OK;
        break;
      }
    case SQLITE3_UTILS_TYPE_BLOB:
      {
        struct sqlite3_utils_blob blob = va_arg(ap, struct sqlite3_utils_blob);
        result = sqlite3_bind_blob(stmt, index, blob.data, blob.length, SQLITE_STATIC);
        break;
      }
    case SQLITE3_UTILS_TYPE_NONE:
      goto done_binding;
    }

    if(result != SQLITE_OK)
    {
      sqlite3_utils_report_error(db);
      rc = -1;
      goto out_clear_bindings;
    }
  }
  done_binding:;

  va_list ap_init;
  va_copy(ap_init, ap);
  for(;;)
    switch(va_arg(ap_init, enum sqlite3_utils_return_type))
    {
    case SQLITE3_UTILS_RETURN_TYPE_INT:
    case SQLITE3_UTILS_RETURN_TYPE_INT64:
    case SQLITE3_UTILS_RETURN_TYPE_DOUBLE:
    case SQLITE3_UTILS_RETURN_TYPE_BLOB:
      break;

    case SQLITE3_UTILS_RETURN_TYPE_ARRAY_INT:
    case SQLITE3_UTILS_RETURN_TYPE_ARRAY_INT64:
    case SQLITE3_UTILS_RETURN_TYPE_ARRAY_DOUBLE:
    case SQLITE3_UTILS_RETURN_TYPE_ARRAY_BLOB:
      *va_arg(ap_init, void **) = NULL;
      break;

    case SQLITE3_UTILS_RETURN_TYPE_NONE:
      goto done_init;
    }
done_init:
  va_end(ap_init);

  for(;;)
    switch(sqlite3_step(stmt))
    {
    case SQLITE_ROW:
      {
        va_list ap_step;
        va_copy(ap_step, ap);
        for(int index = 0; ; ++index)
          switch(va_arg(ap_step, enum sqlite3_utils_return_type))
          {
          case SQLITE3_UTILS_RETURN_TYPE_INT:
            {
              int *value = va_arg(ap_step, int *);
              *value = sqlite3_column_int(stmt, index);
            }
            break;
          case SQLITE3_UTILS_RETURN_TYPE_INT64:
            {
              int64_t *value = va_arg(ap_step, int64_t *);
              *value = sqlite3_column_int64(stmt, index);
            }
            break;
          case SQLITE3_UTILS_RETURN_TYPE_DOUBLE:
            {
              double *value = va_arg(ap_step, double *);
              *value = sqlite3_column_double(stmt, index);
            }
            break;
          case SQLITE3_UTILS_RETURN_TYPE_BLOB:
            {
              struct sqlite3_utils_blob *value = va_arg(ap_step, struct sqlite3_utils_blob *);
              value->data = sqlite3_column_blob(stmt, index);
              value->length = sqlite3_column_bytes(stmt, index);
              value->data = memdup(value->data, value->length);
            }
            break;

          case SQLITE3_UTILS_RETURN_TYPE_ARRAY_INT:
            {
              int **values = va_arg(ap_step, int **);
              int value = sqlite3_column_int(stmt, index);
              arrput(*values, value);
            }
            break;
          case SQLITE3_UTILS_RETURN_TYPE_ARRAY_INT64:
            {
              int64_t **values = va_arg(ap_step, int64_t **);
              int64_t value = sqlite3_column_int64(stmt, index);
              arrput(*values, value);
            }
            break;
          case SQLITE3_UTILS_RETURN_TYPE_ARRAY_DOUBLE:
            {
              double **values = va_arg(ap_step, double **);
              double value = sqlite3_column_double(stmt, index);
              arrput(*values, value);
            }
            break;
          case SQLITE3_UTILS_RETURN_TYPE_ARRAY_BLOB:
            {
              struct sqlite3_utils_blob **values = va_arg(ap_step, struct sqlite3_utils_blob **);
              struct sqlite3_utils_blob value = { .data = sqlite3_column_blob(stmt, index), .length = sqlite3_column_bytes(stmt, index), };
              value.data = memdup(value.data, value.length);
              arrput(*values, value);
            }
            break;
          case SQLITE3_UTILS_RETURN_TYPE_NONE:
            goto done_row;
          }
done_row:
        va_end(ap_step);
      }
      continue;
    case SQLITE_DONE:
      goto done_stepping;
    default:
      sqlite3_utils_report_error(db);
      rc = -1;
      goto out_reset;
    }
done_stepping:

out_reset:
  if(sqlite3_reset(stmt) != SQLITE_OK)
  {
    sqlite3_utils_report_error(db);
    rc = -1;
  }

  if(rc != 0)
  {
    va_list ap_free;
    va_copy(ap_free, ap);
    for(;;)
      switch(va_arg(ap_free, enum sqlite3_utils_return_type))
      {
      case SQLITE3_UTILS_RETURN_TYPE_INT:
      case SQLITE3_UTILS_RETURN_TYPE_INT64:
      case SQLITE3_UTILS_RETURN_TYPE_DOUBLE:
      case SQLITE3_UTILS_RETURN_TYPE_BLOB:
        break;

      case SQLITE3_UTILS_RETURN_TYPE_ARRAY_INT:
      case SQLITE3_UTILS_RETURN_TYPE_ARRAY_INT64:
      case SQLITE3_UTILS_RETURN_TYPE_ARRAY_DOUBLE:
      case SQLITE3_UTILS_RETURN_TYPE_ARRAY_BLOB:
        arrfree(*va_arg(ap_free, void **));
        break;

      case SQLITE3_UTILS_RETURN_TYPE_NONE:
        goto done_free;
      }
done_free:
    va_end(ap_free);
  }

out_clear_bindings:
  sqlite3_clear_bindings(stmt);
  va_end(ap);
  return rc;
}

int sqlite3_utils_exec(sqlite3 *db, const char *sql)
{
  if(sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK)
  {
    sqlite3_utils_report_error(db);
    return -1;
  }
  return 0;
}
