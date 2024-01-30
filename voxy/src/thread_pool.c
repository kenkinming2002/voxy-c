#include "thread_pool.h"

#include <stdlib.h>
#include <stdbool.h>

#include <sys/sysinfo.h>

static void *thread_pool_func(void *arg)
{
  struct thread_pool     *thread_pool = arg;
  struct thread_pool_job *job;
  bool                    shutdown;
  for(;;)
  {
    pthread_mutex_lock(&thread_pool->mutex);
    while(!(shutdown = atomic_load_explicit(&thread_pool->thread_shutdown, memory_order_acquire)) && !thread_pool->head)
      pthread_cond_wait(&thread_pool->cond, &thread_pool->mutex);

    if(shutdown)
    {
      pthread_mutex_unlock(&thread_pool->mutex);
      return NULL;
    }

    job = thread_pool->head;
    thread_pool->head = thread_pool->head->next;
    if(!thread_pool->head)
      thread_pool->tail = NULL;

    pthread_mutex_unlock(&thread_pool->mutex);

    job->func(job->arg);
    free(job->arg);
    free(job);
  }
}

void thread_pool_init(struct thread_pool *thread_pool)
{
  thread_pool->head = NULL;
  thread_pool->tail = NULL;

  pthread_mutex_init(&thread_pool->mutex, NULL);
  pthread_cond_init(&thread_pool->cond, NULL);

  thread_pool->thread_shutdown = false;
  thread_pool->thread_count    = get_nprocs();
  thread_pool->threads         = malloc(thread_pool->thread_count * sizeof *thread_pool->threads);
  for(int i=0; i<thread_pool->thread_count; ++i)
    pthread_create(&thread_pool->threads[i], NULL, &thread_pool_func, thread_pool);
}

void thread_pool_fini(struct thread_pool *thread_pool)
{
  atomic_store_explicit(&thread_pool->thread_shutdown, true, memory_order_release);
  pthread_cond_broadcast(&thread_pool->cond);
  for(int i=0; i<thread_pool->thread_count; ++i)
    pthread_join(thread_pool->threads[i], NULL);
  free(thread_pool->threads);

  pthread_mutex_destroy(&thread_pool->mutex);
  pthread_cond_destroy(&thread_pool->cond);

  struct thread_pool_job *job = thread_pool->head;
  while(job)
  {
    struct thread_pool_job *next = job->next;
    free(job->arg);
    free(job);
    job = next;
  }
}

void thread_pool_enqueue(struct thread_pool *thread_pool, void(*func)(void*), void *arg)
{
  struct thread_pool_job *job = malloc(sizeof *job);
  job->next = NULL;
  job->func = func;
  job->arg  = arg;

  pthread_mutex_lock(&thread_pool->mutex);
  if(thread_pool->tail)
  {
    thread_pool->tail->next = job;
    thread_pool->tail = job;
  }
  else
  {
    thread_pool->head = job;
    thread_pool->tail = job;
  }
  pthread_mutex_unlock(&thread_pool->mutex);
  pthread_cond_signal(&thread_pool->cond);
}
