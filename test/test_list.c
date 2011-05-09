#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "list.h"


#define test(fn) \
        printf("\x1b[33m" # fn "\x1b[0m "); \
        test_##fn(); \
        puts("\x1b[1;32m ✓ \x1b[0m");

int frees = 0;

void
freed(){
  frees++;
}

typedef struct {
  int val;
} wrap_t;

static simplet_list_t*
build_list(){
  simplet_list_t *list;
  if(!(list = simplet_list_new(list)))
    assert(0);
  wrap_t test  = { 5 };
  wrap_t test2 = { 6 };
  wrap_t test3 = { 7 };

  simplet_list_push(list, &test);
  simplet_list_push(list, &test2);
  simplet_list_push(list, &test3);
  return list;
}


static void
test_push(){
  simplet_list_t *list;
  if(!(list = simplet_list_new(list)))
    assert(0);

  assert(list->length == 0);
  wrap_t test = { 5 };
  simplet_list_push(list, &test);
  assert(list->head->value == &test);
  assert(list->tail->value == &test);
  assert(list->length == 1);
  simplet_list_free(list);
}

static void
test_pop(){
  simplet_list_t *list = build_list();

  wrap_t *ret;
  ret = simplet_list_pop(list);
  assert(ret->val == 7);
  ret = simplet_list_pop(list);
  assert(ret->val == 6);
  ret = simplet_list_pop(list);
  assert(ret->val == 5);

  simplet_list_free(list);
}

static void
test_destroy(){
  simplet_list_t *list = build_list();

  list->free = freed;

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

  simplet_list_free(list);
}


int
main(){
  test(push);
  test(pop);
  test(destroy);
  test(iter);
}