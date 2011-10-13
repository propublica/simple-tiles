#include "layer.h"
#include "filter.h"
#include "util.h"
#include "error.h"
#include <cpl_error.h>

// TODO: test for broken copy
simplet_layer_t*
simplet_layer_new(const char *datastring){
  simplet_layer_t *layer;
  if(!(layer = malloc(sizeof(*layer))))
    return NULL;

  layer->source = simplet_copy_string(datastring);

  if(!(layer->filters = simplet_list_new())){
    free(layer);
    return NULL;
  }

  return layer;
}

SIMPLET_ERROR_FUNC(layer_t)

void
simplet_layer_vfree(void *layer){
  simplet_layer_free(layer);
}

void
simplet_layer_free(simplet_layer_t *layer){
  simplet_list_set_item_free(layer->filters, simplet_filter_vfree);
  simplet_list_free(layer->filters);
  free(layer->source);
  free(layer);
}

simplet_filter_t*
simplet_layer_add_filter(simplet_layer_t *layer, const char *ogrsql){
  simplet_filter_t* filter;
  if(!(filter = simplet_filter_new(ogrsql)))
    return NULL;

  if(!simplet_list_push(layer->filters, filter)){
    simplet_filter_free(filter);
    return NULL;
  }

  return filter;
}

simplet_status_t
simplet_layer_process(simplet_layer_t *layer, simplet_map_t *map, cairo_t *ctx){
  simplet_listiter_t *iter; OGRDataSourceH source;
  if(!(source = OGROpenShared(layer->source, 0, NULL)))
    return SIMPLET_OGR_ERR;
  //retain the datasource
  if(OGR_DS_GetRefCount(source) == 1) OGR_DS_Reference(source);
  if(!(iter = simplet_get_list_iter(layer->filters))){
    OGRReleaseDataSource(source);
    return SIMPLET_OOM;
  }

  simplet_filter_t *filter;
  simplet_status_t status = SIMPLET_OK;
  while((filter = simplet_list_next(iter))) {
    status = simplet_filter_process(filter, map, source, ctx);
    if(status != SIMPLET_OK){
      simplet_list_iter_free(iter);
      OGRReleaseDataSource(source);
      return status;
    }
  }
  OGRReleaseDataSource(source);
  return SIMPLET_OK;
}
