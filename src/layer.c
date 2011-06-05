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
  layer->filters->free = simplet_filter_vfree;
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

int
simplet_layer_process(simplet_layer_t *layer, simplet_map_t *map){
  OGRRegisterAll();
  simplet_listiter_t *iter;
  if(!(layer->_source = OGROpen(layer->source, 0, NULL)))
    return 0;

  if(!(iter = simplet_get_list_iter(layer->filters)))
    return 0;

  simplet_filter_t *filter;
  while((filter = simplet_list_next(iter)))
    simplet_filter_process(filter, layer, map);

  OGR_DS_Destroy(layer->_source);

  return 1;
}