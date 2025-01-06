#include "sqlite3_utils.h"

#include <libcore/log.h>

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

#define dynamic_array_append(values, count, capacity, value) \
  do \
  { \
    if((capacity) == (count)) \
    { \
      (capacity) = (capacity) != 0 ? (capacity) * 2 : 1; \
      (values) = realloc((values), (capacity) * sizeof *(values)); \
    } \
    (values)[(count)++] = (value); \
  } \
  while(0)

#define dynamic_array_append2(values1, values2, count, capacity, value1, value2) \
  do \
  { \
    if((capacity) == (count)) \
    { \
      (capacity) = (capacity) != 0 ? (capacity) * 2 : 1; \
      (values1) = realloc((values1), (capacity) * sizeof *(values1)); \
      (values2) = realloc((values2), (capacity) * sizeof *(values2)); \
    } \
    (values1)[(count)] = (value1); \
    (values2)[(count)] = (value2); \
    (count)++; \
  } \
  while(0)

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
        const void *buf = va_arg(ap, void *);
        const size_t len = va_arg(ap, size_t);
        result = sqlite3_bind_blob(stmt, index, buf, len, SQLITE_STATIC);
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
      *va_arg(ap_init, void **) = NULL;
      *va_arg(ap_init, size_t *) = 0;
      *va_arg(ap_init, size_t *) = 0;
      break;

    case SQLITE3_UTILS_RETURN_TYPE_ARRAY_BLOB:
      *va_arg(ap_init, void **) = NULL;
      *va_arg(ap_init, void **) = NULL;
      *va_arg(ap_init, size_t *) = 0;
      *va_arg(ap_init, size_t *) = 0;
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
              const void **buf = va_arg(ap_step, const void **);
              size_t *len = va_arg(ap_step, size_t *);
              *buf = sqlite3_column_blob(stmt, index);
              *len = sqlite3_column_bytes(stmt, index);
              *buf = memdup(*buf, *len);
            }
            break;

          case SQLITE3_UTILS_RETURN_TYPE_ARRAY_INT:
            {
              int **values = va_arg(ap_step, int **);
              size_t *count = va_arg(ap_step, size_t *);
              size_t *capacity = va_arg(ap_step, size_t *);

              int value = sqlite3_column_int(stmt, index);
              dynamic_array_append(*values, *count, *capacity, value);
            }
            break;
          case SQLITE3_UTILS_RETURN_TYPE_ARRAY_INT64:
            {
              int64_t **values = va_arg(ap_step, int64_t **);
              size_t *count = va_arg(ap_step, size_t *);
              size_t *capacity = va_arg(ap_step, size_t *);

              int64_t value = sqlite3_column_int64(stmt, index);
              dynamic_array_append(*values, *count, *capacity, value);
            }
            break;
          case SQLITE3_UTILS_RETURN_TYPE_ARRAY_DOUBLE:
            {
              double **values = va_arg(ap_step, double **);
              size_t *count = va_arg(ap_step, size_t *);
              size_t *capacity = va_arg(ap_step, size_t *);

              double value = sqlite3_column_double(stmt, index);
              dynamic_array_append(*values, *count, *capacity, value);
            }
            break;
          case SQLITE3_UTILS_RETURN_TYPE_ARRAY_BLOB:
            {
              const void ***bufs = va_arg(ap_step, const void ***);
              size_t **lens = va_arg(ap_step, size_t **);
              size_t *count = va_arg(ap_step, size_t *);
              size_t *capacity = va_arg(ap_step, size_t *);

              const void *buf = sqlite3_column_blob(stmt, index);
              size_t len = sqlite3_column_bytes(stmt, index);
              buf = memdup(buf, len);
              dynamic_array_append2(*bufs, *lens, *count, *capacity, buf, len);
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
        free(va_arg(ap_free, void **));
        va_arg(ap_free, size_t *);
        va_arg(ap_free, size_t *);
        break;

      case SQLITE3_UTILS_RETURN_TYPE_ARRAY_BLOB:
        free(va_arg(ap_free, void **));
        free(va_arg(ap_free, void **));
        va_arg(ap_free, size_t *);
        va_arg(ap_free, size_t *);
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
