#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

enum target_type
{
  STATIC_LIBRARY,
  SHARED_LIBRARY,
  EXECUTABLE,
};

struct strings
{
  const char **items;
  size_t count;
};

struct target
{
  const char *path;
  const char *name;

  enum target_type type;
  struct strings sources;
  struct strings dependencies;
  struct strings pkg_configs;

  const char *extra_int_cflags;
  const char *extra_int_libs;
};

struct targets
{
  struct target *items;
  size_t count;
};

struct registry
{
  const char *lower_name;
  const char *upper_name;
};

struct registries
{
  struct registry *items;
  size_t count;
};

#define ARRAY(type, ...) { .items = (type[]){ __VA_ARGS__ }, .count = sizeof (type[]){ __VA_ARGS__ } / sizeof (type[]){ __VA_ARGS__ }[0] }
#define STRINGS(...) ARRAY(const char *, __VA_ARGS__)
#define TARGETS(...) ARRAY(struct target, __VA_ARGS__)
#define REGISTRIES(...) ARRAY(struct registry, __VA_ARGS__)

#define ARRAY_FOREACH(type, array, item) for(type *item = array.items; item < array.items + array.count; ++item)
#define STRINGS_FOREACH(array, item) ARRAY_FOREACH(const char *, array, item)
#define TARGETS_FOREACH(array, item) ARRAY_FOREACH(struct target, array, item)
#define REGISTRIES_FOREACH(array, item) ARRAY_FOREACH(struct registry, array, item)

#define TARGET(...) (struct target){ __VA_ARGS__ }
#define REGISTRY(...) (struct registry){ __VA_ARGS__ }

static char *format(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);

  int n = vsnprintf(NULL, 0, fmt, ap);
  if(n < 0)
    abort();

  va_end(ap);

  char *p = malloc(n+1);
  if(!p)
    abort();

  va_start(ap, fmt);

  if(vsnprintf(p, n+1, fmt, ap) < 0)
    abort();

  va_end(ap);

  return p;
}

static char *read_all(const char *name, FILE *f)
{
  char *p = NULL;
  for(size_t n1 = 0, n2 = 512; ; n1 = n2, n2 = n2 * 2)
  {
    p = realloc(p, n2);

    size_t k = fread(p + n1, 1, n2 - n1, f);

    if(ferror(f))
    {
      fprintf(stderr, "error: failed to read from %s\n", name);
      exit(EXIT_FAILURE);
    }

    if(feof(f))
    {
      p = realloc(p, n1 + k + 1);
      p[n1 + k] = '\0';
      return p;
    }
  }
}

static char *query_pkg_config(const char *module, const char *query)
{
  char *command = format("pkg-config --%s %s", query, module);;
  FILE *process = popen(command, "r");
  if(!process)
  {
    fprintf(stderr, "error: failed to execute pkg-config: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  char *result = read_all("pkg-config", process);
  for(char *p = result; *p; ++p)
    if(isspace(*p))
      *p = ' ';

  pclose(process);
  free(command);

  return result;
}

int main()
{
  struct targets targets = TARGETS(
    TARGET(
      .name = "external",
      .path = "lib/external",
      .type = STATIC_LIBRARY,
      .sources = STRINGS(
        "stb_ds",
      ),
    ),
    TARGET(
      .name = "core",
      .path = "lib/core",
      .type = STATIC_LIBRARY,
      .sources = STRINGS(
        "profile",
        "thread_pool",
        "fs",
      ),
    ),
    TARGET(
      .name = "math",
      .path = "lib/math",
      .type = STATIC_LIBRARY,
      .sources = STRINGS(
        "dummy",
      ),
    ),
    TARGET(
      .name = "serde",
      .path = "lib/serde",
      .type = STATIC_LIBRARY,
      .sources = STRINGS(
        "serializer",
        "deserializer",
      ),
    ),
    TARGET(
      .name = "net",
      .path = "lib/net",
      .type = STATIC_LIBRARY,
      .sources = STRINGS(
        "client",
        "server",
      ),
      .pkg_configs = STRINGS(
        "openssl",
      ),
    ),
    TARGET(
      .name = "gfx",
      .path = "lib/gfx",
      .type = STATIC_LIBRARY,
      .sources = STRINGS(
        "mesh",
        "delta_time",
        "glad",
        "stb_image",
        "render",
        "window",
        "font_set",
        "gl",
        "time",
        "camera",
      ),
      .dependencies = STRINGS(
        "external",
        "core",
        "math",
      ),
      .pkg_configs = STRINGS(
        "glfw3",
        "fontconfig",
        "freetype2",
      ),
    ),
    TARGET(
      .name = "ui",
      .path = "lib/ui",
      .type = STATIC_LIBRARY,
      .dependencies = STRINGS(
        "gfx",
      ),
      .sources = STRINGS(
        "ui",
      ),
    ),
    TARGET(
      .name = "voxy_server",
      .path = "bin/voxy/server",
      .type = EXECUTABLE,
      .dependencies = STRINGS(
        "external",
        "core",
        "math",
        "serde",
        "net",
      ),
      .sources = STRINGS(
        "sqlite3_utils",
        "registry/entity_info",
        "registry/block",
        "registry/entity",
        "registry/item",
        "light/manager",
        "physics/physics",
        "physics/swept",
        "chunk/coordinates",
        "chunk/manager",
        "chunk/block/database",
        "chunk/block/network",
        "chunk/block/group",
        "chunk/block/generator",
        "chunk/block/manager",
        "chunk/entity/database",
        "chunk/entity/network",
        "chunk/entity/entity",
        "chunk/entity/allocator",
        "chunk/entity/manager",
        "application",
        "main",
        "player/player",
        "player/manager",
        "mod/mod",
        "mod/manager",
      ),
      .pkg_configs = STRINGS(
        "sqlite3",
        "liburing",
      ),
      .extra_int_cflags = "-Ibin/voxy/config/include -Ibin/voxy/protocol/include",
      .extra_int_libs = "-lm",
    ),
    TARGET(
      .name = "voxy_client",
      .path = "bin/voxy/client",
      .type = EXECUTABLE,
      .dependencies = STRINGS(
        "external",
        "core",
        "math",
        "serde",
        "net",
        "gfx",
        "ui",
      ),
      .sources = STRINGS(
        "camera/manager",
        "input/manager",
        "registry/block",
        "registry/entity",
        "registry/item",
        "render/block/mesh",
        "render/block/block",
        "render/block/render_info",
        "render/world",
        "render/entity/entity",
        "chunk/block/group",
        "chunk/block/manager",
        "chunk/entity/entity",
        "chunk/entity/manager",
        "application",
        "main",
        "mod/mod",
        "mod/manager",
        "ui/manager",
      ),
      .extra_int_cflags = "-Ibin/voxy/config/include -Ibin/voxy/protocol/include",
      .extra_int_libs = "-lm",
    ),
    TARGET(
      .name = "base_server",
      .path = "bin/mod/base/server",
      .type = SHARED_LIBRARY,
      .sources = STRINGS("mod"),
      .dependencies = STRINGS("voxy_server"),
      .extra_int_cflags = "-Ibin/voxy/config/include",
    ),
    TARGET(
      .name = "base_client",
      .path = "bin/mod/base/client",
      .type = SHARED_LIBRARY,
      .sources = STRINGS("mod"),
      .dependencies = STRINGS("voxy_client"),
      .extra_int_cflags = "-Ibin/voxy/config/include",
    ),
    TARGET(
      .name = "render_block",
      .path = "bin/tools/render_block",
      .type = EXECUTABLE,
      .sources = STRINGS("render_block"),
      .dependencies = STRINGS(
        "core",
        "gfx",
      ),
      .pkg_configs = STRINGS(
        "libpng"
      ),
      .extra_int_libs = "-lm",
    ),
  );

  struct strings systems = STRINGS("server", "client");
  struct registries registries = REGISTRIES(
    REGISTRY( .lower_name = "entity", .upper_name = "ENTITY", ),
    REGISTRY( .lower_name = "block",  .upper_name = "BLOCK",  ),
    REGISTRY( .lower_name = "item",   .upper_name = "ITEM",   ),
  );


  FILE *build = fopen("build.ninja", "w");
  if(!build)
  {
    fprintf(stderr, "error: failed to open build.ninja: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  const char *cc = getenv("CC");
  if(!cc) cc = "cc";

  const char *ar = getenv("AR");
  if(!ar) ar = "ar";

  const char *cflags = getenv("CFLAGS");
  if(!cflags) cflags = "";

  const char *libs = getenv("LIBS");
  if(!libs) libs = "";

  fprintf(build,
    "cc = %s\n"
    "ar = %s\n"
    "\n"
    "global_cflags = -Wall -Wextra -D_GNU_SOURCE -I abomination %s\n"
    "global_libs = %s\n"
    "\n"
    "rule static_library\n"
    "  command = $ar rcs $out $objects\n"
    "\n"
    "rule executable\n"
    "  command = $cc $cflags -o $out $objects $libs\n"
    "\n"
    "rule shared_library\n"
    "  command = $cc -shared $cflags -o $out $objects $libs\n"
    "\n"
    "rule cc\n"
    "  command = $cc -MD -MF $out.d $cflags -c -o $out $in\n"
    "  depfile = $out.d\n"
    "\n"
    "rule sed_registry\n"
    "  command = sed -e s/name/$lower_name/g -e s/NAME/$upper_name/g < $in > $out\n"
    "\n"
    , cc, ar, cflags, libs
  );

  fprintf(build,
    "rule rebuild\n"
    "  command = $cc -o build build.c && CC=\"%s\" CFLAGS=\"%s\" ./build\n"
    "build build.ninja: rebuild build.c"
    "\n",
    cc, cflags);

  STRINGS_FOREACH(systems, system) {
    REGISTRIES_FOREACH(registries, registry) {
      fprintf(build,
        "generated_sources =$generated_sources bin/voxy/%1$s/include/voxy/%1$s/registry/%2$s.h\n"
        "build bin/voxy/%1$s/include/voxy/%1$s/registry/%2$s.h: sed_registry bin/voxy/%1$s/include/voxy/%1$s/registry/registry.h\n"
        "  lower_name=%2$s\n"
        "  upper_name=%3$s\n"
        "\n"
        "generated_sources =$generated_sources build bin/voxy/%1$s/src/registry/%2$s.h\n"
        "build bin/voxy/%1$s/src/registry/%2$s.h: sed_registry bin/voxy/%1$s/src/registry/registry.h\n"
        "  lower_name=%2$s\n"
        "  upper_name=%3$s\n"
        "\n"
        "generated_sources =$generated_sources build bin/voxy/%1$s/src/registry/%2$s.c\n"
        "build bin/voxy/%1$s/src/registry/%2$s.c: sed_registry bin/voxy/%1$s/src/registry/registry.c\n"
        "  lower_name=%2$s\n"
        "  upper_name=%3$s\n"
        "\n",
        *system, registry->lower_name, registry->upper_name);
    }
  }

  fprintf(build, "build generated_sources: phony");
  STRINGS_FOREACH(systems, system) {
    REGISTRIES_FOREACH(registries, registry) {
      fprintf(build,
        " bin/voxy/%1$s/include/voxy/%1$s/registry/%2$s.h"
        " build bin/voxy/%1$s/src/registry/%2$s.h"
        " build bin/voxy/%1$s/src/registry/%2$s.c",
        *system, registry->lower_name);
    }
  }
  fprintf(build, "\n\n");


  TARGETS_FOREACH(targets, target) {
    // out
    fprintf(build, "%s_out_name = ", target->name);
    switch(target->type)
    {
    case STATIC_LIBRARY:
      fprintf(build, "lib%s.a", target->name);
      break;
    case EXECUTABLE:
      fprintf(build, "%s", target->name);
      break;
    case SHARED_LIBRARY:
      fprintf(build, "%s.so", target->name);
      break;
    }
    fprintf(build, "\n");
    fprintf(build, "%2$s_out = %1$s/$%2$s_out_name\n", target->path, target->name);
    fprintf(build, "\n");

    // cflags
    {
      fprintf(build, "%1$s_cflags = -I %2$s/include", target->name, target->path);

      STRINGS_FOREACH(target->dependencies, dependency) {
        fprintf(build, " $%s_cflags", *dependency);
      }

      STRINGS_FOREACH(target->pkg_configs, pkg_config) {
        char *cflags = query_pkg_config(*pkg_config, "cflags");
        fprintf(build, " %s", cflags);
        free(cflags);
      }

      fprintf(build, "\n");
    }

    // int_cflags
    {
      fprintf(build, "%1$s_int_cflags = $global_cflags $%1$s_cflags -I %2$s/src", target->name, target->path);

      if(target->type == SHARED_LIBRARY)
        fprintf(build, " -fPIC");

      if(target->extra_int_cflags)
        fprintf(build, " %s", target->extra_int_cflags);

      fprintf(build, "\n");
    }

    // libs
    {
      fprintf(build, "%s_libs =", target->name);

      // While I do not see any problem with linking with an executable that is
      // compiled with -rdynamic and export its symbol, it is not allowed and we
      // have to check for that.
      if(target->type != EXECUTABLE)
        fprintf(build, " -L %1$s -l:$%2$s_out_name", target->path, target->name);

      // If we use dynamic linking, we can link with our dependencies ourself
      // and do not need to propagate it downstream.
      if(target->type != SHARED_LIBRARY && target->type != EXECUTABLE) {
        STRINGS_FOREACH(target->dependencies, dependency) {
          fprintf(build, " $%s_libs", *dependency);
        }

        STRINGS_FOREACH(target->pkg_configs, pkg_config) {
          char *libs = query_pkg_config(*pkg_config, "libs");
          fprintf(build, " %s", libs);
          free(libs);
        }

        if(target->extra_int_libs)
          fprintf(build, " %s", target->extra_int_libs);
      }

      fprintf(build, "\n");
    }

    // int_libs
    {
      // We can only link with our dependencies ourself if we use dynamic
      // linking. There is no way to supply linker flags for static archive, and
      // we need to rely on whoever link the static archive to link our
      // dependencies correctly.
      if(target->type == SHARED_LIBRARY || target->type == EXECUTABLE) {
        fprintf(build, "%s_int_libs = $global_libs", target->name);

        if(target->type == EXECUTABLE)
          fprintf(build, " -fvisibility=hidden -rdynamic");

        STRINGS_FOREACH(target->dependencies, dependency) {
          fprintf(build, " $%s_libs", *dependency);
        }

        STRINGS_FOREACH(target->pkg_configs, pkg_config) {
          char *libs = query_pkg_config(*pkg_config, "libs");
          fprintf(build, " %s", libs);
          free(libs);
        }

        if(target->extra_int_libs)
          fprintf(build, " %s", target->extra_int_libs);
      }

      fprintf(build, "\n");
    }

    // sources
    {
      STRINGS_FOREACH(target->sources, source) {
        fprintf(build,
          "build %1$s/src/%3$s.o: cc %1$s/src/%3$s.c || generated_sources\n"
          "  cflags = $%2$s_int_cflags -I %1$s/src\n"
          "  libs = $%2$s_int_libs\n",
          target->path, target->name, *source
        );
      }
      fprintf(build, "\n");
    }

    // link
    {
      fprintf(build, "build $%s_out: ", target->name);
      switch(target->type)
      {
      case STATIC_LIBRARY:
        fprintf(build, "static_library");
        break;
      case SHARED_LIBRARY:
        fprintf(build, "shared_library");
        break;
      case EXECUTABLE:
        fprintf(build, "executable");
        break;
      }

      STRINGS_FOREACH(target->sources, source) {
        fprintf(build, " %1$s/src/%2$s.o", target->path, *source);
      }

      // We only need to wait for our dependencies to compile if we are actually
      // going to link with them. This is not the case for static library.
      if(target->type == SHARED_LIBRARY || target->type == EXECUTABLE) {
        STRINGS_FOREACH(target->dependencies, dependency) {
          fprintf(build, " $%1$s_out", *dependency);
        }
      }

      fprintf(build, "\n");

      fprintf(build, "  objects =");
      STRINGS_FOREACH(target->sources, source) {
        fprintf(build, " %1$s/src/%2$s.o", target->path, *source);
      }
      fprintf(build, "\n");

      fprintf(build,
        "  cflags = $%1$s_int_cflags\n"
        "  libs = $%1$s_int_libs\n"
        "\n",
        target->name
      );
    }
  }

  fclose(build);
}
