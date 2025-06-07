#ifndef LIBCORE_DS_RING_BUFFER_H
#define LIBCORE_DS_RING_BUFFER_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Implementation of a dynamic ring buffer in the style of stb_ds. */

#define rb_initial_cap 32

#define rb_head(t) ((t) ? rb_header(t)->head : 0)
#define rb_tail(t) ((t) ? rb_header(t)->tail : 0)
#define rb_cap(t)  ((t) ? rb_header(t)->capacity : 0)

#define rb_push(t,v) ((t) = rb_maybe_grow((t), sizeof *(t)), (t)[rb_header(t)->head++ & (rb_header(t)->capacity - 1)] = v)
#define rb_pop(t)    ((t)[rb_header(t)->tail++ & (rb_header(t)->capacity - 1)])

#define rb_header(t) ((struct rb_header *)(t) - 1)

struct rb_header
{
  size_t capacity;
  size_t head;
  size_t tail;
};

static inline void *rb_maybe_grow(void *t, size_t elemsize)
{
  if(!t)
  {
    struct rb_header *h = malloc(sizeof *h + elemsize * rb_initial_cap);
    h->capacity = rb_initial_cap;
    h->head = 0;
    h->tail = 0;
    return h + 1;
  }

  struct rb_header *h = rb_header(t);
  if(h->tail + h->capacity != h->head)
    return t;

  size_t old_capacity = h->capacity;
  h->capacity *= 2;
  h = realloc(h, sizeof *h + h->capacity * elemsize);
  t = h + 1;
  memcpy((char *)t + old_capacity * elemsize, t, old_capacity * elemsize);

  return t;
}

#endif // LIBCORE_DS_RING_BUFFER_H
