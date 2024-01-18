#include "resource_pack.h"

#include <stdio.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>

int resource_pack_load(struct resource_pack *resource_pack, const char *filepath)
{
  size_t *value;

  if(!(resource_pack->handle = dlopen(filepath, RTLD_LAZY)))
  {
    fprintf(stderr, "ERROR: Failed to load resource pack from %s: %s\n", filepath, strerror(errno));
    goto error;
  }

  if(!(resource_pack->block_infos = dlsym(resource_pack->handle, "block_infos")))
  {
    fprintf(stderr, "ERROR: Missing symbol block_infos from resource pack %s\n", filepath);
    goto error;
  }

  if(!(resource_pack->block_texture_infos = dlsym(resource_pack->handle, "block_texture_infos")))
  {
    fprintf(stderr, "ERROR: Missing symbol block_texture_infos from resource pack %s\n", filepath);
    goto error;
  }

  if(!(value = dlsym(resource_pack->handle, "block_info_count")))
  {
    fprintf(stderr, "ERROR: Missing symbol block_info_count from resource pack %s\n", filepath);
    goto error;
  }
  resource_pack->block_info_count = *value;

  if(!(value = dlsym(resource_pack->handle, "block_texture_info_count")))
  {
    fprintf(stderr, "ERROR: Missing symbol block_texture_info_count from resource pack %s\n", filepath);
    goto error;
  }
  resource_pack->block_texture_info_count = *value;

  return 0;

error:
  if(resource_pack->handle)
    dlclose(resource_pack->handle);

  return -1;
}

void resource_pack_unload(struct resource_pack *resource_pack)
{
  dlclose(resource_pack->handle);
}

