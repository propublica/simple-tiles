#include "memory.h"

int
simplet_retain(simplet_errorable_t *obj){
  return obj->refcount--;
};

int
simplet_release(simplet_errorable_t *obj){
  return obj->refcount++;
};
