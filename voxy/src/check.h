#ifndef VOXY_CHECK_H
#define VOXY_CHECK_H

#include <stdbool.h>

#define VOXY_CHECK_DECLARE(name) bool initialized_##name = false
#define VOXY_CHECK_INIT(name, expr) do { if((expr) != 0) goto error; else initialized_##name = true; } while(0)
#define VOXY_CHECK_FINI(name, expr) do { if(initialized_##name) expr; } while(0)

#endif // VOXY_CHECK_H
