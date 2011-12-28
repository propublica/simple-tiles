#include "user_data.h"

void
simplet_set_user_data(simplet_with_user_data_t *obj, void *data){
  obj->user_data = data;
}

void *
simplet_get_user_data(simplet_with_user_data_t *obj){
  return obj->user_data;
}

void
simplet_free_user_data(simplet_with_user_data_t *obj, simplet_user_data_free free_data){
  free_data(obj->user_data);
}
