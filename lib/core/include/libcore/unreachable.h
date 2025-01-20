#ifndef LIBCORE_UNREACHABLE_H
#define LIBCORE_UNREACHABLE_H

#include <assert.h>

#define LIBCORE_UNREACHABLE do { assert(0 && "unreachable"); } while(0)

#endif // LIBCORE_UNREACHABLE_H
