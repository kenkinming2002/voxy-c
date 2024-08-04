#include <voxy/core/thread_pool.h>

#include <stdlib.h>
#include <stdbool.h>

#include <sys/sysinfo.h>

static struct thread_pool_job *head;
static struct thread_pool_job *tail;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;

static int         thread_count;
static pthread_t  *threads;
static atomic_bool thread_shutdown;

static void *thread_pool_func()
{
  struct thread_pool_job *job;
  bool                    shutdown;
  for(;;)
  {
    pthread_mutex_lock(&mutex);
    while(!(shutdown = atomic_load_explicit(&thread_shutdown, memory_order_acquire)) && !head)
      pthread_cond_wait(&cond, &mutex);

    if(shutdown)
    {
      pthread_mutex_unlock(&mutex);
      return NULL;
    }

    job = head;
    head = head->next;
    if(!head)
      tail = NULL;

    pthread_mutex_unlock(&mutex);
    job->invoke(job);
  }
}

static void thread_pool_atexit(void)
{
  atomic_store_explicit(&thread_shutdown, true, memory_order_release);
  pthread_cond_broadcast(&cond);
  for(int i=0; i<thread_count; ++i)
    pthread_join(threads[i], NULL);
  free(threads);

  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);

  struct thread_pool_job *job = head;
  while(job)
  {
    struct thread_pool_job *next = job->next;
    job->destroy(job);
    job = next;
  }
}

static void thread_pool_ensure_init(void)
{
  if(!threads)
  {
    thread_count = get_nprocs();
    threads      = malloc(thread_count * sizeof *threads);
    for(int i=0; i<thread_count; ++i)
      pthread_create(&threads[i], NULL, &thread_pool_func, NULL);

    atexit(thread_pool_atexit);
  }
}

void thread_pool_enqueue(struct thread_pool_job *job)
{
  thread_pool_ensure_init();

  job->next = NULL;
  pthread_mutex_lock(&mutex);
  if(tail)
  {
    tail->next = job;
    tail = job;
  }
  else
  {
    head = job;
    tail = job;
  }
  pthread_mutex_unlock(&mutex);
  pthread_cond_signal(&cond);
}

