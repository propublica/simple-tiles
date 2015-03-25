#include "test.h"
#include "list.h"


int frees = 0;

typedef struct {
  int val;
} wrap_t;

void
free_wrap(void *value){
  wrap_t *val = value;
  free(val);
}

void
freed(void *value){
  frees++;
  free_wrap(value);
}

wrap_t*
wrap_new(int val){
  wrap_t* wrap;
  assert(wrap = malloc(sizeof(*wrap)));
  wrap->val = val;
  return wrap;
}

static simplet_list_t*
build_list(){
  simplet_list_t *list;
  if(!(list = simplet_list_new()))
    assert(0);

  wrap_t *test = wrap_new(5);
  wrap_t *test2 = wrap_new(6);
  wrap_t *test3 = wrap_new(7);

  simplet_list_push(list, test);
  simplet_list_push(list, test2);
  simplet_list_push(list, test3);
  return list;
}

static void
test_get(){
  simplet_list_t *list = build_list();
  wrap_t *test = simplet_list_get(list, 0);
  assert(test->val == 5);
  test = simplet_list_get(list, list->length);
  assert(!test);
  simplet_list_set_item_free(list, free_wrap);
  simplet_list_free(list);
}

static void
test_push(){
  simplet_list_t *list;
  if(!(list = simplet_list_new()))
    assert(0);
  assert(list->length == 0);
  wrap_t *test = wrap_new(5);
  simplet_list_push(list, test);
  assert(simplet_list_head(list) == test);
  assert(simplet_list_tail(list) == test);
  assert(list->length == 1);
  simplet_list_set_item_free(list, free_wrap);
  simplet_list_free(list);
}

static void
test_pop(){
  simplet_list_t *list = build_list();
  wrap_t *ret;
  ret = simplet_list_pop(list);
  assert(ret->val == 7);
  free(ret);
  ret = simplet_list_pop(list);
  assert(ret->val == 6);
  free(ret);
  ret = simplet_list_pop(list);
  assert(ret->val == 5);
  free(ret);
  simplet_list_set_item_free(list, free_wrap);
  simplet_list_free(list);
}

static void
test_destroy(){
  simplet_list_t *list = build_list();
  simplet_list_set_item_free(list, freed);
  simplet_list_free(list);
  assert(frees == 3);
}

static void
test_iter(){
  simplet_list_t *list = build_list();
  simplet_listiter_t *iter = simplet_get_list_iter(list);
  wrap_t *ret;
  assert((ret = simplet_list_next(iter))->val == 5);
  assert((ret = simplet_list_next(iter))->val == 6);
  assert((ret = simplet_list_next(iter))->val == 7);
  assert(simplet_list_next(iter) == NULL);
  simplet_list_set_item_free(list, free_wrap);
  simplet_list_free(list);
}

TASK(list) {
  test(push);
  test(pop);
  test(get);
  test(destroy);
  test(iter);
}
