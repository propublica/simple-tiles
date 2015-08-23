#ifndef _SIMPLE_USER_DATA_H
#define _SIMPLE_USER_DATA_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void simplet_set_user_data(simplet_with_user_data_t *obj, void *data);

void *simplet_get_user_data(simplet_with_user_data_t *obj);

void simplet_free_user_data(simplet_with_user_data_t *obj,
                            simplet_user_data_free free);

#define SIMPLET_HAS_USER_DATA_PROTOS(type)                                  \
  void simplet_##type##_set_user_data(simplet_##type##_t *obj, void *data); \
  void *simplet_##type##_get_user_data(simplet_##type##_t *obj);            \
  void simplet_##type##_free_user_data(simplet_##type##_t *obj,             \
                                       simplet_user_data_free free);

#define SIMPLET_HAS_USER_DATA(type)                                          \
  void simplet_##type##_set_user_data(simplet_##type##_t *obj, void *data) { \
    simplet_set_user_data((simplet_with_user_data_t *)obj, data);            \
  }                                                                          \
  void *simplet_##type##_get_user_data(simplet_##type##_t *obj) {            \
    return simplet_get_user_data((simplet_with_user_data_t *)obj);           \
  }                                                                          \
  void simplet_##type##_free_user_data(simplet_##type##_t *obj,              \
                                       simplet_user_data_free free) {        \
    simplet_free_user_data((simplet_with_user_data_t *)obj, free);           \
  }

#ifdef __cplusplus
}
#endif

#endif
