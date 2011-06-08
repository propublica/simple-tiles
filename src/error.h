#ifndef _SIMPLE_TILES_ERROR_H
#define _SIMPLE_TILES_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

void
simplet_set_error_handle(void (*handle)(simplet_error_t err, char *mess));

void
simplet_error(simplet_error_t err, char *mess);

simplet_status_t
simplet_check_cairo(cairo_t *ctx);

void
simplet_cairo_error(cairo_t *ctx);

void
simplet_ogr_error();

void
simplet_get_last_error();


#ifdef __cplusplus
}
#endif

#endif