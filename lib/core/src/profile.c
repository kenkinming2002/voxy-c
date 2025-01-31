#include <libcore/profile.h>

#include <libcore/log.h>
#include <libcore/format.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <errno.h>
#include <time.h>
#include <unistd.h>

static void profile_append_event(const char *name, const char *cat, const char *ph, va_list args)
{
  static FILE *file = NULL;
  static const char *separator = "[\n";

  static bool initialized;
  if(!initialized)
  {
    initialized = true;

    const char *profile_name;
    if(!(profile_name = getenv("PROFILE")))
    {
      LOG_INFO("Environmental variable PROFILE not set: Profiling disabled");
      return;
    }

    if(!(file = fopen(profile_name, "w")))
    {
      LOG_WARN("Failed to open file: %s", strerror(errno));
      return;
    }
  }

  if(!file)
    return;

  fprintf(file, "%s", separator);
  separator = ",\n";

  // TODO: We are using clock_get_time() which is non-portable instead of our
  //       own get_time() function.
  //
  //       This is because our current implementation of get_time() functio
  //       calls glfwGetTime() internally. However, we do not have any graphics
  //       on the server and hence we did not even bother with initializing
  //       GLFW. This means that calls glfwGetTime() will always return 0.0 to
  //       indicate an error which again we are not even checking.
  //
  //       We need to implement get_time() without depending on GLFW, and fix
  //       here to use it instead of using clock_get_time() directly.
  struct timespec time;
  if(clock_gettime(CLOCK_MONOTONIC, &time) == -1)
  {
    LOG_WARN("Failed to get time: %s", strerror(errno));
    return;
  }

  const long long nsec = time.tv_sec * 1000000000 + time.tv_nsec;

  const pid_t pid = getpid();
  const pid_t tid = gettid();

  fprintf(file, "{");
  {
    fprintf(file, "\"name\":\"%s\"", name);
    fprintf(file, ",");
    fprintf(file, "\"cat\":\"%s\"", cat);
    fprintf(file, ",");
    fprintf(file, "\"ph\":\"%s\"", ph);

    fprintf(file, ",");
    fprintf(file, "\"ts\":\"%lf\"", (double)nsec * 1e-3);

    fprintf(file, ",");
    fprintf(file, "\"pid\":\"%d\"", pid);
    fprintf(file, ",");
    fprintf(file, "\"tid\":\"%d\"", tid);

    fprintf(file, ",");
    fprintf(file, "\"args\":{");

    const char *key, *value;
    bool initial = true;

    while((key = va_arg(args, const char *)) && (value = va_arg(args, const char *)))
    {
      if(initial)
        initial = false;
      else
        fprintf(file, ",");

      fprintf(file, "\"%s\":\"%s\"", key, value);
    }

    fprintf(file, "}");
  }
  fprintf(file, "}");
}

void profile_begin_impl(const char *name, ...)
{
  va_list ap;
  va_start(ap, name);
  profile_append_event(name, "core", "B", ap);
  va_end(ap);
}

void profile_end_impl(const char *name, ...)
{
  va_list ap;
  va_start(ap, name);
  profile_append_event(name, "core", "E", ap);
  va_end(ap);
}

void profile_scope_begin(const struct profile_scope_info *info)
{
  profile_begin_impl(tformat("%s:%d", info->function, info->line), NULL);
}

void profile_scope_end(const struct profile_scope_info *info)
{
  profile_end_impl(tformat("%s:%d", info->function, info->line), NULL);
}

