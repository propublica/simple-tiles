#ifndef _SIMPLE_TILES_ERROR_H
#define _SIMPLE_TILES_ERROR_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SIMPLET_ERROR_FUNC(type) \
static simplet_status_t \
set_error(simplet_##type *item, simplet_status_t status, const char *msg) { \
  return simplet_error((simplet_errorable_t *)item, status, msg); \
}

int
simplet_set_error(simplet_errorable_t *error, simplet_status_t status, const char *msg);

void
simplet_error_init();

simplet_status_t
simplet_error(simplet_errorable_t *errr, simplet_status_t err, const char* msg);

#ifdef __cplusplus
}
#endif

#endif
