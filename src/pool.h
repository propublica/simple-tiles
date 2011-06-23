#ifndef _SIMPLET_POOL_H
#define _SIMPLET_POOL_H

#include <pthread.h>
#include "types.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*simplet_pool_worker)(void *unit);

typedef enum {
  SIMPLET_RUN, SIMPLET_EXIT
} simplet_pool_state_t;

typedef struct {
  simplet_list_t       *work;
  simplet_listiter_t   *iter;
  simplet_pool_worker  worker;
  simplet_pool_state_t status;
  pthread_mutex_t       lock;
  pthread_t             *threads;
  int                    live;
  int                   size;
} simplet_pool_t;



simplet_pool_t*
simplet_pool_new();

void
simplet_pool_free(simplet_pool_t *pool, void (*destroy)(void *val));

void
simplet_pool_set_work(simplet_pool_t *pool, simplet_list_t *work);

void
simplet_pool_set_worker(simplet_pool_t *pool, simplet_pool_worker worker);

void
simplet_pool_start(simplet_pool_t *pool);

void
simplet_pool_set_size(simplet_pool_t *pool, int size);

#ifdef __cplusplus
}
#endif

#endif
