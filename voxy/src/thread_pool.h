#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <stdatomic.h>

struct thread_pool_job
{
  struct thread_pool_job *next;

  void(*func)(void *);
  void *arg;
};

struct thread_pool
{
  struct thread_pool_job *head;
  struct thread_pool_job *tail;

  pthread_mutex_t mutex;
  pthread_cond_t  cond;

  atomic_bool thread_shutdown;
  int         thread_count;
  pthread_t  *threads;
};

void thread_pool_init(struct thread_pool *thread_pool);
void thread_pool_fini(struct thread_pool *thread_pool);
void thread_pool_enqueue(struct thread_pool *thread_pool, void(*func)(void *), void *arg);

#endif // THREAD_POOL_H
