#include <pthread.h>
#include "pool.h"
#include "assert.h"

#define SIMPLET_THREADS 4

static void elision(void *val) { (void) val; }

simplet_pool_t*
simplet_pool_new(){
  simplet_pool_t *pool;
  if(!(pool = malloc(sizeof(*pool))))
    return NULL;

  if((pthread_mutex_init(&pool->lock, NULL) > 0)){
    free(pool);
    return NULL;
  }

  pool->work    = NULL;
  pool->threads = NULL;
  pool->worker  = &elision;
  pool->size    = SIMPLET_THREADS;
  pool->live    = 0;
  pool->status  = SIMPLET_EXIT;
  pool->iter    = NULL;

  return pool;
}

void
simplet_pool_free(simplet_pool_t *pool, void (*destroy)(void *val)){
  pthread_mutex_destroy(&pool->lock);
  if(pool->work){
    if(destroy) simplet_list_set_item_free(pool->work, destroy);
    simplet_list_free(pool->work);
  }
  free(pool);
}

void
simplet_pool_set_work(simplet_pool_t *pool, simplet_list_t *work){ // todo: error handling
  pool->work = work;
  pool->iter = simplet_get_list_iter(work);
}

void
simplet_pool_set_size(simplet_pool_t *pool, int size){
  if(!size > SIMPLET_THREADS)
    pool->size = size;
}

void
simplet_pool_set_worker(simplet_pool_t *pool, simplet_pool_worker worker){
  pool->worker = worker;
}

static void*
perform(void *threadpool){ /* needs error handling */
  simplet_pool_t *pool = threadpool;
  pthread_mutex_lock(&pool->lock);
  ++pool->live;
  pthread_mutex_unlock(&pool->lock);
  // crazy times please fix
  for(;;){
    pthread_mutex_lock(&pool->lock);
    if(pool->status == SIMPLET_EXIT) {
      pthread_mutex_unlock(&pool->lock);
      break;
    }
    void *unit = simplet_list_next(pool->iter);
    if(!unit) {
      pool->status = SIMPLET_EXIT;
      pthread_mutex_unlock(&pool->lock);
      break;
    }
    pthread_mutex_unlock(&pool->lock);
    if(pool->status == SIMPLET_RUN) pool->worker(unit);
  }

  pthread_mutex_lock(&pool->lock);
  --pool->live;
  pthread_mutex_unlock(&pool->lock);
  pthread_exit(NULL);
  return NULL;
}

void
simplet_pool_start(simplet_pool_t *pool){ /* blocks */
  if(pool->status == SIMPLET_RUN) return;
  if(!(pool->threads = (pthread_t *) malloc(sizeof(pthread_t) * pool->size)))
    return;

  pool->status = SIMPLET_RUN;
  for(int i = 0; i < pool->size; i++)
    pthread_create(&pool->threads[i], NULL, perform, (void *) pool);
  for(int i = 0; i < pool->size; i++)
    pthread_join(pool->threads[i], NULL);

  memset(pool->threads, 0, pool->size * sizeof(pthread_t));
  free(pool->threads);
  pool->threads = NULL;
  assert(pool->live == 0);
}
