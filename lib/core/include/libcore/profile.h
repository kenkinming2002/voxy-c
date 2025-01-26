#ifndef LIBCORE_PROFILE_H
#define LIBCORE_PROFILE_H

/// Implementation of a basic profiling system inspired by [VISUAL BENCHMARKING
/// in C++ (how to measure performance visually) - YouTube](
/// https://www.youtube.com/watch?v=xlAH4dbMVnU>).
///
/// If PROFILE environmental variable is set, profiling output is written to
/// file at PROFILE with chrome trace event format. The output can be opened by
/// navigating to chrome://tracing in any chromium-based browser.
///
/// The problem of performance tuning in game developement is different from
/// other domains. The amount of time spent on some particular functions in
/// total does not matter as much as the maximum amount of time you could
/// potentially spent in a single function call. This is where conventional
/// profiling tool like perf falls short.
///
/// Let say you are targeting 60fps, then you have about 16.6ms to spent each
/// frame for your game logic and rendering. If you go over it, you will get
/// just a visual stutter. You *CANNOT* compensate for it by spending fewer time
/// next frame.

#include <stdlib.h>

#define profile_begin(...) profile_begin_impl(__FUNCTION__, ##__VA_ARGS__, NULL)
#define profile_end(...) profile_end_impl(__FUNCTION__, ##__VA_ARGS__, NULL)

void profile_begin_impl(const char *name, ...);
void profile_end_impl(const char *name, ...);

struct profile_scope_info
{
  const char *function;
  int line;
};

void profile_scope_begin(const struct profile_scope_info *info);
void profile_scope_end(const struct profile_scope_info *info);

#define profile_scope \
  const struct profile_scope_info profile_scope_info __attribute__((cleanup(profile_scope_end))) = { .function = __FUNCTION__, .line = __LINE__, }; \
  profile_scope_begin(&profile_scope_info);

#endif // LIBCORE_PROFILE_H
