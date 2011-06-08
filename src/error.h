#ifndef _SIMPLE_TILES_ERROR_H
#define _SIMPLE_TILES_ERROR_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*simplet_error_handler)(simplet_status_t err, char *mess);

void
simplet_set_error_handle(simplet_error_handler handle);

void
simplet_error(simplet_status_t err);

simplet_status_t
simplet_check_cairo(cairo_t *ctx);

void
simplet_cairo_error(cairo_t *ctx);

void
simplet_ogr_error();

#ifdef __cplusplus
}
#endif

#endif