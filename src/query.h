#ifndef _SIMPLE_TILES_QUERY_H
#define _SIMPLE_TILES_QUERY_H

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
simplet_query_vfree(void *query);

void
simplet_query_free(simplet_query_t *query);

simplet_query_t*
simplet_query_new(const char *sqlquery);

simplet_status_t
simplet_query_set(simplet_query_t *query, const char *sql);

simplet_status_t
simplet_query_get(simplet_query_t *query, char **sql);

simplet_style_t*
simplet_query_add_style(simplet_query_t *query, const char *key, const char *arg);

simplet_style_t*
simplet_query_add_style_directly(simplet_query_t *query, simplet_style_t *style);

simplet_status_t
simplet_query_process(simplet_query_t *query, simplet_map_t *map,
  OGRDataSourceH source, simplet_lithograph_t *litho, cairo_t *ctx);

SIMPLET_HAS_USER_DATA_PROTOS(query)

#ifdef __cplusplus
}
#endif

#endif
