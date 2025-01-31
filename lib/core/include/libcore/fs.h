#ifndef LIBCORE_FS_H
#define LIBCORE_FS_H

#include <stddef.h>

/// Directory separator.
///
/// FIXME: Do we want to support windows directory separator which can also be \.
#define DIRECTORY_SEPARATOR '/'

/// Recursive mkdir.
///
/// Return non-zero value on failure.
int mkdir_recursive(const char *dir);

/// Create hard link.
///
/// Return non-zero value on failure.
int create_hard_link(const char *path, const char *target);

/// Rename a file.
///
/// Return non-zero value on failure.
int file_rename(const char *from, const char *to);

/// Remove a file.
///
/// Return non-zero value on failure.
int file_remove(const char *path);

/// Return parent directory of dir.
///
/// This literally just search for the last directory separator and return a
/// duplicate of the substring up to that not including the separator.
char *parent(const char *dir);

/// Read all data from a file.
///
/// Return non-zero value on failure.
int read_file_all(const char *path, char **data, size_t *length);

/// Write all data to a file.
///
/// Return non-zero value on failure.
int write_file_all(const char *path, const char *data, size_t length);

#endif // LIBCORE_FS_H
