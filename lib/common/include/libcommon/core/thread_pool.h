#ifndef LIBCOMMON_CORE_THREAD_POOL_H
#define LIBCOMMON_CORE_THREAD_POOL_H

#include <pthread.h>
#include <stdatomic.h>

struct thread_pool_job
{
  struct thread_pool_job *next;

  void(*invoke)(struct thread_pool_job *);
  void(*destroy)(struct thread_pool_job *);
};

void thread_pool_enqueue(struct thread_pool_job *job);

#endif // LIBCOMMON_CORE_THREAD_POOL_H
