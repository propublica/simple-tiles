#ifndef _SIMPLE_TILES_filter_H
#define _SIMPLE_TILES_filter_H

#include "types.h"
#include "list.h"
#include "map.h"
#include "style.h"
#include "text.h"
#include "user_data.h"

#ifdef __cplusplus
extern "C" {
#endif

void
simplet_filter_vfree(void *filter);

void
simplet_filter_free(simplet_filter_t *filter);

simplet_filter_t*
simplet_filter_new(const char *sqlquery);

simplet_status_t
simplet_filter_set_query(simplet_filter_t *filter, const char *query);

simplet_status_t
simplet_filter_get_query(simplet_filter_t *filter, char **query);

simplet_style_t*
simplet_filter_add_style(simplet_filter_t *filter, const char *key, const char *arg);

simplet_status_t
simplet_filter_process(simplet_filter_t *filter, simplet_map_t *map,
  OGRDataSourceH source, simplet_lithograph_t *litho, cairo_t *ctx);

SIMPLET_HAS_USER_DATA_PROTOS(filter)

#ifdef __cplusplus
}
#endif

#endif
