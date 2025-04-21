#ifndef ABOMINATION_STDLIB_H
#define ABOMINATION_STDLIB_H

#include_next <stdlib.h>

static inline void qsort_safe(void *base, size_t nmemb, size_t size, int (*compar) (const void *, const void *)) __attribute__((nonnull(4)));
static inline void qsort_safe(void *base, size_t nmemb, size_t size, int (*compar) (const void *, const void *))
{
  if(nmemb != 0)
    qsort(base, nmemb, size, compar);
}

#define qsort qsort_safe

#endif // ABOMINATION_STDLIB_H
