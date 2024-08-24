#ifndef LIBCOMMON_CORE_FS_H
#define LIBCOMMON_CORE_FS_H

/// Directory separator.
///
/// FIXME: Do we want to support windows directory separator which can also be \.
#define DIRECTORY_SEPARATOR '/'

/// Recursive mkdir.
///
/// This will modify dir in the process. Like mkdir(), this will return 0 on
/// success. On error, -1 is returned and errno is set to indicate the error.
int mkdir_recursive(char *dir);

/// Return parent directory of dir.
///
/// This literally just search for the last directory separator and return a
/// duplicate of the substring up to that not including the separator.
char *parent(const char *dir);

#endif // LIBCOMMON_CORE_FS_H
