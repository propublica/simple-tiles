#include "test.h"
#include <simple-tiles/pool.h>

typedef struct wrap_t {
  int value;
} wrap_t;

static void
free_wrap(void *wrap){
  free((wrap_t*) wrap);
}

static simplet_list_t*
build_list(){
  simplet_list_t *wraps;
  assert((wraps = simplet_list_new()));
  simplet_list_set_item_free(wraps, free_wrap);
  for(int i = 0; i < 5; i++){
    wrap_t *wrap;
    assert((wrap = malloc(sizeof(*wrap))));
    wrap->value = i + 1;
    simplet_list_push(wraps, wrap);
  }
  return wraps;
}

static void
worker(void *value){
  wrap_t *wrap = value;
  wrap->value = wrap->value * 2;
}

static void
test_pool(){
  simplet_list_t *work = build_list();
  simplet_pool_t *pool;
  assert((pool = simplet_pool_new()));
  simplet_pool_set_work(pool, work);
  simplet_pool_set_worker(pool, worker);
  simplet_pool_start(pool);

  wrap_t *it = work->head->value;
  assert(it->value == 2);
  it = work->tail->value;
  assert(it->value == 10);
  simplet_list_free(work);
}

TASK(pool){
  test(pool);
}

