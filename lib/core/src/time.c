#include <libcore/time.h>

#include <time.h>
#include <math.h>
#include <errno.h>

double time_get(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void time_sleep(double secs)
{
  struct timespec ts;
  ts.tv_sec = floorf(secs);
  ts.tv_nsec = floorf((secs - floor(secs))* 1e9);
  nanosleep(&ts, NULL);
}

static struct timespec ts;

void time_sleep_interruptible_begin(double secs)
{
  ts.tv_sec = floorf(secs);
  ts.tv_nsec = floorf((secs - floor(secs))* 1e9);
}

bool time_sleep_interruptible(void)
{
  return nanosleep(&ts, &ts) == -1 && errno == EINTR;
}

