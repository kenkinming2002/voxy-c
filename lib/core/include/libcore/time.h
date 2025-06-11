#ifndef LIBCORE_TIME_H
#define LIBCORE_TIME_H

#include <stdbool.h>

double time_get(void);
void time_sleep(double secs);

void time_sleep_interruptible_begin(double secs);
bool time_sleep_interruptible(void);

#endif // LIBCORE_TIME_H
