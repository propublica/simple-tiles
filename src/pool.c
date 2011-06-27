#include <pthread.h>
#include "pool.h"
#include "assert.h"

#define SIMPLET_THREADS 4
int elision(void *unit) { (void) unit; return 0; }

simplet_pool_t*
simplet_pool_new(){
  simplet_pool_t *pool;
  if(!(pool = malloc(sizeof(*pool))))
    return NULL;

  if((pthread_mutex_init(&pool->lock, NULL) > 0)){
    free(pool);
    return NULL;
  }

  pool->work       = NULL;
  pool->threads    = NULL;
  pool->worker     = elision;
  pool->size       = SIMPLET_THREADS;
  pool->error_code = SIMPLET_OK;
  pool->live       = 0;
  pool->status     = SIMPLET_EXIT;
  pool->iter       = NULL;

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

static void
cleanup(void *pool){
  simplet_pool_t *p = pool;
  pthread_mutex_lock(&p->lock);
  --p->live;
  pthread_mutex_unlock(&p->lock);
}

static void*
perform(void *threadpool){ /* needs error handling */
  simplet_pool_t *pool = threadpool;
  pthread_mutex_lock(&pool->lock);
  ++pool->live;
  pthread_mutex_unlock(&pool->lock);
  pthread_cleanup_push(cleanup, (void *)pool);
  // crazy times please fix
  for(;;){
    pthread_testcancel();
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
    if(pool->status == SIMPLET_RUN) {
      int err = pool->worker(unit);
      if(err != SIMPLET_OK){
        pthread_mutex_lock(&pool->lock);
        pool->error_code = err;
        pthread_mutex_unlock(&pool->lock);
        break;
      }
    }
  }
  pthread_cleanup_pop(1);
  pthread_exit(NULL);
  return NULL;
}

int
simplet_pool_start(simplet_pool_t *pool){ /* blocks */
  if(pool->status == SIMPLET_RUN) return SIMPLET_ERR;
  if(!(pool->threads = (pthread_t *) malloc(sizeof(pthread_t) * pool->size)))
    return SIMPLET_OOM;

  pool->status = SIMPLET_RUN;
  int err = SIMPLET_OK;
  for(int i = 0; i < pool->size; i++)
    pthread_create(&pool->threads[i], NULL, perform, (void *) pool);
  for(int j = 0; j < pool->size; j++){
    if(err == SIMPLET_OK){
      pthread_join(pool->threads[j], NULL);
    } else {
      pthread_cancel(pool->threads[j]);
    }
  }

  memset(pool->threads, 0, pool->size * sizeof(pthread_t));
  free(pool->threads);
  pool->threads = NULL;
  assert(pool->live == 0);
  return pool->error_code;
}
