#include <libcommon/core/fs.h>

#include <sys/stat.h>
#include <errno.h>

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

