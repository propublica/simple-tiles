#include "user_data.h"

// Set the user data on a generic obj.
void
simplet_set_user_data(simplet_with_user_data_t *obj, void *data){
  obj->user_data = data;
}

// Return a pointer to the user data stored in the generic obj.
void *
simplet_get_user_data(simplet_with_user_data_t *obj){
  return obj->user_data;
}

// Free user data and set the field to NULL.
void
simplet_free_user_data(simplet_with_user_data_t *obj, simplet_user_data_free free_data){
  free_data(obj->user_data);
}
