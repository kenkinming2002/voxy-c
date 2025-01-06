#include <libcore/fs.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/// Recursive mkdir.
///
/// Return non-zero value on failure.
int mkdir_recursive(const char *_dir)
{
  int result = 0;

  char *dir = strdup(_dir);
  for(char *p = dir; *p; ++p)
    if(*p == DIRECTORY_SEPARATOR)
    {
      *p = '\0';
      result = mkdir(dir, 0755);
      *p = DIRECTORY_SEPARATOR;
      if(result != 0 && errno != EEXIST)
        goto out;
    }

  result = mkdir(dir, 0755);
  if(result != 0 && errno != EEXIST)
    goto out;

out:
  free(dir);
  return result;
}

/// Create hard link.
///
/// Return non-zero value on failure.
int create_hard_link(const char *path, const char *target)
{
  return link(target, path);
}

/// Rename a file.
///
/// Return non-zero value on failure.
int file_rename(const char *from, const char *to)
{
  return rename(from, to);
}

/// Remove a file.
///
/// Return non-zero value on failure.
int file_remove(const char *path)
{
  return unlink(path);
}

char *parent(const char *dir)
{
  size_t n = 0;
  for(size_t i = 0; dir[i] != '\0'; ++i)
    if(dir[i] == DIRECTORY_SEPARATOR)
      n = i;

  char *parent = malloc(n+1);
  memcpy(parent, dir, n);
  parent[n] = '\0';
  return parent;
}
