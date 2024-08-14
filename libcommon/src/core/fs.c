#include <libcommon/core/fs.h>

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/stat.h>

/// Recursive mkdir.
///
/// This will modify dir in the process.
int mkdir_recursive(char *dir)
{
  for(char *p = dir; *p; ++p)
    if(*p == DIRECTORY_SEPARATOR)
    {
      *p = '\0';
      int result = mkdir(dir, 0755);
      *p = DIRECTORY_SEPARATOR;
      if(result != 0 && errno != EEXIST)
        return -1;
    }

  int result = mkdir(dir, 0755);
  if(result != 0 && errno != EEXIST)
    return -1;

  return 0;
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
