#include "layer.h"
#include "vector_layer.h"
#include "raster_layer.h"
#include "util.h"
#include "error.h"
#include <cpl_error.h>
#include "memory.h"

// Add in an error function.
SIMPLET_ERROR_FUNC(layer_t)

// Free a void pointer pointing to a layer instance.
void
simplet_layer_vfree(void *layer){
  simplet_layer_t *cast_layer = (simplet_layer_t *) layer;
  if (cast_layer->type == SIMPLET_VECTOR) {
    simplet_vector_layer_free((simplet_vector_layer_t *) cast_layer);
  } else if (cast_layer->type == SIMPLET_RASTER) {
    simplet_raster_layer_free((simplet_raster_layer_t *) cast_layer);
  }
}

// Get the datasource string for this layer.
void
simplet_layer_get_source(simplet_layer_t *layer, char **source){
  *source = simplet_copy_string(layer->source);
}

// Set a copy of this source as this layers datasource string.
void
simplet_layer_set_source(simplet_layer_t *layer, char *source){
  char *src = simplet_copy_string(source);
  if(!src) set_error(layer, SIMPLET_OOM, "out of memory setting source");
  layer->source = src;
}
