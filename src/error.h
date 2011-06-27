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


#ifdef __cplusplus
}
#endif

#endif
