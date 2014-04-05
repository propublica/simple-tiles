#include "test.h"
#include "raster_layer.h"

void
test_raster_layer(){
  simplet_raster_layer_t *layer;
  if(!(layer = simplet_raster_layer_new("../data/loss_1932_2010.tif")))
    assert(0);
  assert(layer->source);
  assert(layer->type == SIMPLET_RASTER);
  simplet_raster_layer_free(layer);
}

TASK(raster_layer){
  test(raster_layer);
}