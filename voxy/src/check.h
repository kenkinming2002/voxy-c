#ifndef VOXY_CHECK_H
#define VOXY_CHECK_H

#include <stdbool.h>

#define VOXY_CHECK_DECLARE(name) bool initialized_##name = false
#define VOXY_CHECK_INIT(name, expr) do { if((expr) != 0) goto error; else initialized_##name = true; } while(0)
#define VOXY_CHECK_FINI(name, expr) do { if(initialized_##name) expr; } while(0)

#define VOXY_CHECK_ARRAY_DECLARE(name) size_t initialized_count_##name = 0;
#define VOXY_CHECK_ARRAY_INIT(name, expr, count) do { for(; initialized_count_##name<(count); ++initialized_count_##name) { size_t i = initialized_count_##name; if((expr) != 0) goto error; } } while(0)
#define VOXY_CHECK_ARRAY_FINI(name, expr) do { for(size_t i=0; i<initialized_count_##name; ++i) expr; } while(0)

#define VOXY_FINI(expr) do { expr; } while(0)
#define VOXY_ARRAY_FINI(expr, count) do { for(size_t i=0; i<(count); ++i) expr; } while(0)

#endif // VOXY_CHECK_H
