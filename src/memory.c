#include "memory.h"

int
simplet_retain(simplet_retainable_t *obj){
  obj->refcount++;
  return obj->refcount;
}

int
simplet_release(simplet_retainable_t *obj){
  obj->refcount--;
  return obj->refcount;
}
