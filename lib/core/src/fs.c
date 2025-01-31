#include <libcore/fs.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>

/// Recursive mkdir.
///
/// Return non-zero value on failure.
int mkdir_recursive(const char *_dir)
{
  char *dir = strdupa(_dir);
  for(char *p = dir; *p; ++p)
    if(*p == DIRECTORY_SEPARATOR)
    {
      *p = '\0';

      if(mkdir(dir, 0755) != 0 && errno != EEXIST)
        return -1;

      *p = DIRECTORY_SEPARATOR;
    }

  if(mkdir(dir, 0755) != 0 && errno != EEXIST)
    return -1;

  return 0;
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

static int read_all(int fd, char *buf, size_t n)
{
  while(n > 0)
  {
    ssize_t result = read(fd, buf, n);
    if(result == -1)
    {
      if(errno == EINTR)
        continue;
      return -1;
    }

    buf += result;
    n -= result;
  }
  return 0;
}

static int write_all(int fd, const char *buf, size_t n)
{
  while(n > 0)
  {
    ssize_t result = write(fd, buf, n);
    if(result == -1)
    {
      if(errno == EINTR)
        continue;
      return -1;
    }

    buf += result;
    n -= result;
  }
  return 0;
}

/// Read all data from a file.
///
/// Return non-zero value on failure.
int read_file_all(const char *path, char **data, size_t *length)
{
  int fd = open(path, O_RDONLY);
  if(fd == -1)
    return -1;

  struct stat stat;
  if(fstat(fd, &stat) == -1)
  {
    close(fd);
    return -1;
  }

  *length = stat.st_size;
  *data = malloc(*length);
  if(read_all(fd, *data, *length) != 0)
  {
    free(*data);
    close(fd);
    return -1;
  }

  close(fd);
  return 0;
}

/// Write all data to a file.
///
/// Return non-zero value on failure.
int write_file_all(const char *path, const char *data, size_t length)
{
  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd == -1)
    return -1;

  int result = write_all(fd, data, length);
  close(fd);
  return result;
}

