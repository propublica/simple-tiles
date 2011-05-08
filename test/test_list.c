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
freed(void *val){
  frees++;
}


static void
test_push(){
  simplet_list_t *list;
  if(!(list = simplet_list_new(list)))
    assert(0);

  assert(list->length == 0);
  int test = 5; 
  simplet_list_push(list, &test);
  assert(*(int *)list->head->value == test);
  assert(*(int *)list->tail->value == test);
  assert(list->length == 1);  
  simplet_list_free(list);
}

static void
test_pop(){
  simplet_list_t *list;
  if(!(list = simplet_list_new(list)))
    assert(0);
  int test = 5; 
  int test2 = 6; 
  int test3 = 7; 
  
  simplet_list_push(list, &test);
  simplet_list_push(list, &test2);
  simplet_list_push(list, &test3);
  
  int ret;
  ret = *(int *)simplet_list_pop(list);
  assert(ret == 7);
  ret = *(int *)simplet_list_pop(list);
  assert(ret == 6);
  ret = *(int *)simplet_list_pop(list);
  assert(ret == 5);
  
  simplet_list_free(list);
}

static void
test_destroy(){
  simplet_list_t *list;
  if(!(list = simplet_list_new(list)))
    assert(0);
  int test = 5; 
  int test2 = 6; 
  int test3 = 7; 
  
  simplet_list_push(list, &test);
  simplet_list_push(list, &test2);
  simplet_list_push(list, &test3);
  
  list->free = freed;
  
  simplet_list_free(list);
  assert(frees == 3);
}

static void
test_iter(){
}


int
main(){
  test(push);
  test(pop);
  test(destroy);
  test(iter);  
}