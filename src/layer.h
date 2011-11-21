#ifndef _SIMPLET_LAYER_H
#define _SIMPLET_LAYER_H

#include "types.h"
#include "text.h"

#ifdef __cplusplus
extern "C" {
#endif

simplet_layer_t*
simplet_layer_new(const char *datastring);

void
simplet_layer_vfree(void *layer);

void
simplet_layer_free(simplet_layer_t *layer);

simplet_filter_t*
simplet_layer_add_filter(simplet_layer_t *layer, const char *ogrsql);

simplet_status_t
simplet_layer_process(simplet_layer_t *layer, simplet_map_t *map, simplet_lithograph_t *litho, cairo_t *ctx);

#ifdef __cplusplus
}
#endif

#endif
