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

#endif // LIBCOMMON_CORE_FS_H
