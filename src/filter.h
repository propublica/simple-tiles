#ifndef _SIMPLE_TILES_filter_H
#define _SIMPLE_TILES_filter_H

#include "types.h"
#include "list.h"
#include "map.h"
#include "style.h"

#ifdef __cplusplus
extern "C" {
#endif

void
simplet_filter_vfree(void *filter);

void
simplet_filter_free(simplet_filter_t *filter);

simplet_filter_t*
simplet_filter_new(const char *sqlquery);

simplet_style_t*
simplet_filter_add_style(simplet_filter_t *filter, const char *key, const char *arg);

int
simplet_filter_process(simplet_filter_t *filter, simplet_layer_t *layer, simplet_map_t *map);

#ifdef __cplusplus
}
#endif

#endif
