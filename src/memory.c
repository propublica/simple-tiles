#include "memory.h"

int
simplet_retain(simplet_retainable_t *obj){
  return obj->refcount++;
};

int
simplet_release(simplet_retainable_t *obj){
  return obj->refcount--;
};
