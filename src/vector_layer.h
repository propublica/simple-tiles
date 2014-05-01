#ifndef _SIMPLET_VECTOR_LAYER_H
#define _SIMPLET_VECTOR_LAYER_H

#include "types.h"
#include "text.h"
#include "user_data.h"


#ifdef __cplusplus
extern "C" {
#endif

simplet_vector_layer_t*
simplet_vector_layer_new(const char *datastring);

void
simplet_vector_layer_vfree(void *layer);

void
simplet_vector_layer_free(simplet_vector_layer_t *layer);

simplet_query_t*
simplet_vector_layer_add_query(simplet_vector_layer_t *layer, const char *ogrsql);

simplet_query_t*
simplet_vector_layer_add_query_directly(simplet_vector_layer_t *layer, simplet_query_t *query);

simplet_status_t
simplet_vector_layer_process(simplet_vector_layer_t *layer, simplet_map_t *map, simplet_lithograph_t *litho, cairo_t *ctx);

void
simplet_vector_layer_get_source(simplet_vector_layer_t *layer, char **source);

void
simplet_vector_layer_set_source(simplet_vector_layer_t *layer, char *source);

SIMPLET_HAS_USER_DATA_PROTOS(vector_layer)


#ifdef __cplusplus
}
#endif

#endif
