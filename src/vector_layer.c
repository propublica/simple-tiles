#include <string.h>
#include <cpl_error.h>

#include "vector_layer.h"
#include "query.h"
#include "util.h"
#include "error.h"
#include "memory.h"

// Set up user data.
SIMPLET_HAS_USER_DATA(vector_layer)

// Create and return a new layer instance.
simplet_vector_layer_t *simplet_vector_layer_new(const char *datastring) {
  simplet_vector_layer_t *layer;
  if (!(layer = malloc(sizeof(*layer)))) return NULL;

  memset(layer, 0, sizeof(*layer));

  layer->source = simplet_copy_string(datastring);
  layer->type = SIMPLET_VECTOR;
  layer->status = SIMPLET_OK;

  if (!(layer->queries = simplet_list_new())) {
    free(layer);
    return NULL;
  }

  simplet_retain((simplet_retainable_t *)layer);
  return layer;
}

// Add in an error function.
SIMPLET_ERROR_FUNC(vector_layer_t)

// Free a void pointer pointing to a layer instance.
void simplet_vector_layer_vfree(void *layer) {
  simplet_vector_layer_free(layer);
}

// Free a layer object, and associated layers.
void simplet_vector_layer_free(simplet_vector_layer_t *layer) {
  if (simplet_release((simplet_retainable_t *)layer) > 0) return;
  if (layer->error_msg) free(layer->error_msg);

  simplet_list_set_item_free(layer->queries, simplet_query_vfree);
  simplet_list_free(layer->queries);
  free(layer->source);
  free(layer);
}

// Creat and append a query to the layer's queries.
simplet_query_t *simplet_vector_layer_add_query(simplet_vector_layer_t *layer,
                                                const char *ogrsql) {
  simplet_query_t *query;
  if (!(query = simplet_query_new(ogrsql))) return NULL;

  if (!simplet_list_push(layer->queries, query)) {
    simplet_query_free(query);
    return NULL;
  }

  return query;
}

// Add a previously initialized query to the layer.
simplet_query_t *simplet_vector_layer_add_query_directly(
    simplet_vector_layer_t *layer, simplet_query_t *query) {
  if (!simplet_list_push(layer->queries, query)) return NULL;
  return query;
}

// Process a layer and add labels.
simplet_status_t simplet_vector_layer_process(simplet_vector_layer_t *layer,
                                              simplet_map_t *map,
                                              simplet_lithograph_t *litho,
                                              cairo_t *ctx) {
  simplet_listiter_t *iter;
  OGRDataSourceH source;
  if (!(source = OGROpenShared(layer->source, 0, NULL)))
    return set_error(layer, SIMPLET_OGR_ERR, "error opening layer source");

  // Retain the datasource because we want to cache open connections to a
  // data source like postgres.
  if (OGR_DS_GetRefCount(source) == 1) OGR_DS_Reference(source);
  if (!(iter = simplet_get_list_iter(layer->queries))) {
    OGRReleaseDataSource(source);
    return set_error(layer, SIMPLET_OOM, "out of memory getting list iterator");
  }

  // Loop through the layer's queries and process them.
  simplet_query_t *query;
  simplet_status_t status = SIMPLET_OK;
  while ((query = simplet_list_next(iter))) {
    status = simplet_query_process(query, map, source, litho, ctx);

    if (status != SIMPLET_OK) {
      simplet_list_iter_free(iter);
      OGRReleaseDataSource(source);
      return set_error(layer, query->status, query->error_msg);
    }

    simplet_lithograph_apply(litho, query->styles);
  }
  OGRReleaseDataSource(source);
  return SIMPLET_OK;
}
