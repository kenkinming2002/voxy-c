#ifndef SQLITE3_UTILS_H
#define SQLITE3_UTILS_H

#include <sqlite3.h>

enum sqlite3_utils_type
{
  SQLITE3_UTILS_TYPE_NONE,

  SQLITE3_UTILS_TYPE_INT,
  SQLITE3_UTILS_TYPE_INT64,
  SQLITE3_UTILS_TYPE_DOUBLE,
  SQLITE3_UTILS_TYPE_BLOB,
};

enum sqlite3_utils_return_type
{
  SQLITE3_UTILS_RETURN_TYPE_NONE,

  SQLITE3_UTILS_RETURN_TYPE_INT,
  SQLITE3_UTILS_RETURN_TYPE_INT64,
  SQLITE3_UTILS_RETURN_TYPE_DOUBLE,
  SQLITE3_UTILS_RETURN_TYPE_BLOB,

  SQLITE3_UTILS_RETURN_TYPE_ARRAY_INT,
  SQLITE3_UTILS_RETURN_TYPE_ARRAY_INT64,
  SQLITE3_UTILS_RETURN_TYPE_ARRAY_DOUBLE,
  SQLITE3_UTILS_RETURN_TYPE_ARRAY_BLOB,
};

/// Prepare a sqlite statement.
///
/// Basically the same as sqlite3_prepare_v2() without the shenanigan.
///
/// This is a no-op if *stmt is non-null. The intent is for *stmt to be declared
/// with static storage duration and initialized on first use.
///
/// Return non-zero on error.
int sqlite3_utils_prepare_once(sqlite3 *db, sqlite3_stmt **stmt, const char *sql);

/// Run a sqlite statement.
///
/// Variadic arguments are used to specify both:
///  - values to be bound to the sql query
///  - storage for return values of the sql query
///
/// Values to be bound to the sql query are specified by first passing in a
/// constant from enum sqlite_utils_type to specify its type, followed by the
/// actual value.
///
/// For SQLITE3_UTILS_TYPE_BLOB, both the data pointer and the data length of
/// type size_t need to be passed in.
///
/// The end of the specification is indicated by the sentinel
/// SQLITE3_UTILS_TYPE_NONE.
///
/// This is to be followed by the storage for return values of the sql query,
/// which are specified by first passing in a constant from enum
/// sqlite3_utils_return_type to specify its type, followed by a pointer to the
/// storage location of the actual value.
///
/// For SQLITE3_UTILS_RETURN_TYPE_BLOB, pointer to storage location for both
/// the data pointer and data length of type size_t need to be passed in. The
/// returned buffer need to be freed with a call to free(3).
///
/// For SQLITE3_UTILS_RETURN_TYPE_ARRAY_*, the arguments of be passed in a
/// pointer to the storage location of data pointer(s), count and capacity
/// found in a dynamic array. The data pointer need to be freed with a call to
/// free(3).
///
/// The end of the specification is indicated by the sentinel
/// SQLITE3_UTILS_RETURN_TYPE_NONE.
///
/// See usage in entity/database.c for concrete examples.
///
/// Return non-zero on error.
int sqlite3_utils_run(sqlite3 *db, sqlite3_stmt *stmt, ...);

/// Exec a sqlite statement.
///
/// This is basically the same as sqlite3_exec(). This combines
/// sqlite3_utils_prepare_once() and sqlite3_utils_run() in a single function
/// call at the expense of losing the ability to bind individural arguments and
/// slight performance loss as the same sql code need to parsed and compiled
/// every time this function is called. This makes it a perfect fit for
/// scenarios in which the sql code will only be executed once anyway, such as
/// creating tables during initialization.
///
/// Return non-zero on error.
int sqlite3_utils_exec(sqlite3 *db, const char *sql);

#endif // SQLITE3_UTILS_H
