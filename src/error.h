#ifndef _SIMPLE_TILES_ERROR_H
#define _SIMPLE_TILES_ERROR_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void
simplet_set_error(simplet_error_t *error, simplet_status_t status, const char *msg);

void
simplet_error_init();

simplet_status_t
simplet_error(simplet_errorable_t *errr, simplet_status_t err, const char* msg);

#ifdef __cplusplus
}
#endif

#endif
