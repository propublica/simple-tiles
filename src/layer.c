#include "layer.h"
#include "vector_layer.h"
#include "raster_layer.h"
#include "util.h"
#include "error.h"
#include <cpl_error.h>
#include "memory.h"

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