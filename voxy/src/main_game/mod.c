#include <main_game/mod.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

static void *mod_dl;

#define X(name) static const struct name *mod_##name##s;
MOD_ARRAYS
#undef X

#define X(name) static uint8_t mod_##name##_count;
MOD_ARRAYS
#undef X

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

#define X(name) { \
    void *data  = dlsym(mod_dl, #name "s");                                           \
    void *count = dlsym(mod_dl, #name "_count");                                      \
    if(data && count)                                                                 \
    {                                                                                 \
      mod_##name##s      = data;                                                      \
      mod_##name##_count = *(size_t *)count;                                          \
    }                                                                                 \
    else                                                                              \
      fprintf(stderr, "WARNING: Failed to load " #name " array from %s\n", filepath); \
  }

MOD_ARRAYS
#undef X

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

#define X(name) const struct name *mod_##name##_get(uint8_t id) { return &mod_##name##s[id]; }
MOD_ARRAYS
#undef X

#define X(name) uint8_t mod_##name##_count_get() { return mod_##name##_count; }
MOD_ARRAYS
#undef X

void mod_generate_blocks(seed_t seed, ivec3_t position, uint8_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH]) { return mod_generate_blocks_ptr(seed, position, blocks); }
fvec3_t mod_generate_spawn(seed_t seed) { return mod_generate_spawn_ptr(seed); }
