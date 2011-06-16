#include "layer.h"
#include "filter.h"
#include "util.h"
#include "pool.h"

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

typedef struct _state {
	simplet_filter_t *filter;
	simplet_layer_t  *layer;
	simplet_map_t		 *map;
} _state;

static _state*
state_new(simplet_map_t *map, simplet_layer_t *layer, simplet_filter_t *filter){
	_state *state;
	if(!(state = malloc(sizeof(*state))))
		return NULL;
	state->map    = map;
	state->layer  = layer;
	state->filter = filter;
	return state;
}

static void
state_free(void *state){
	_state *tmp = state;
	free(tmp);
}

static void
process_filter(void *state){
	_state *tmp = state;
	simplet_filter_process(tmp->filter, tmp->layer, tmp->map);
}

simplet_status_t
simplet_layer_process(simplet_layer_t *layer, simplet_map_t *map){
	simplet_listiter_t *iter;
  if(!(iter = simplet_get_list_iter(layer->filters))) goto bail3;

	simplet_pool_t *pool;
	if(!(pool = simplet_pool_new())) goto bail2;

	simplet_pool_set_worker(pool, process_filter);
	simplet_pool_set_size(pool, layer->filters->length);

	simplet_list_t *work;
	if(!(work = simplet_list_new())) goto bail1;

	simplet_filter_t *filter;
  while((filter = simplet_list_next(iter))){
		_state *state;
		if(!(state = state_new(map, layer, filter))) goto bail;
		simplet_list_push(work, state);
	}
	
	simplet_pool_set_work(pool, work);
	simplet_pool_start(pool);
	simplet_pool_free(pool, state_free);
  return SIMPLET_OK;

bail:
	simplet_list_free(work);
bail1:
	simplet_pool_free(pool, state_free);
bail2:
	simplet_list_iter_free(iter);
bail3:
	return SIMPLET_OOM;
}
