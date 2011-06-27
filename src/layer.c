#include "layer.h"
#include "filter.h"
#include "util.h"

simplet_layer_t*
simplet_layer_new(const char *datastring){
  simplet_layer_t *layer;
  if(!(layer = malloc(sizeof(*layer))))
    return NULL;

  layer->_source = NULL;
  layer->source = simplet_copy_string(datastring);

  if(!(layer->filters = simplet_list_new())){
    free(layer);
    return NULL;
  }

  return layer;
}

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
simplet_layer_process(simplet_layer_t *layer, simplet_map_t *map){
  simplet_listiter_t *iter;
  if(!(layer->_source = OGROpen(layer->source, 0, NULL)))
    return SIMPLET_OGR_ERR;

  if(!(iter = simplet_get_list_iter(layer->filters))){
    OGR_DS_Destroy(layer->_source);
    return SIMPLET_OOM;
  }

  simplet_filter_t *filter;
  simplet_status_t status = SIMPLET_OK;
  while((filter = simplet_list_next(iter))) {
    status = simplet_filter_process(filter, layer, map);
    if(status != SIMPLET_OK){
      OGR_DS_Destroy(layer->_source);
      simplet_list_iter_free(iter);
      return status;
    }
  }

  OGR_DS_Destroy(layer->_source);

  return SIMPLET_OK;
}
