#include <voxy/main_game/mod.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

static void *mod_dl;

#define X(name, ret, ...) static ret (*mod_##name##_ptr)(__VA_ARGS__);
MOD_FUNCTIONS
#undef X

static void mod_atexit(void)
{
  dlclose(mod_dl);
}

void mod_load(const char *filepath)
{
  if(mod_dl)
  {
    fprintf(stderr, "WARNING: Attempted to load mod from %s.\n", filepath);
    fprintf(stderr, "WARNING: Loading multiple mods is not supported.\n");
    return;
  }

  mod_dl = dlopen(filepath, RTLD_LAZY);
  if(!mod_dl)
  {
    fprintf(stderr, "WARNING: Failed to load mod from %s.\n", filepath);
    return;
  }
  atexit(mod_atexit);

  void(*mod_init)(void) = dlsym(mod_dl, "mod_init");
  if(mod_init)
    mod_init();

#define X(name, ret, ...) {                                                              \
    void *function = dlsym(mod_dl, #name);                                               \
    if(function)                                                                         \
      mod_##name##_ptr = function;                                                       \
    else                                                                                 \
      fprintf(stderr, "WARNING: Failed to load " #name " function from %s\n", filepath); \
  }

MOD_FUNCTIONS
#undef X
}

void mod_generate_blocks(seed_t seed, ivec3_t position, block_id_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH]) { return mod_generate_blocks_ptr(seed, position, blocks); }
fvec3_t mod_generate_spawn(seed_t seed) { return mod_generate_spawn_ptr(seed); }
