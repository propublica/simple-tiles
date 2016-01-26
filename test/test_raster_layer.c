#include "test.h"
#include "raster_layer.h"

static void test_raster_layer() {
  simplet_raster_layer_t *layer;
  if (!(layer = simplet_raster_layer_new("./data/loss_1932_2010.tif")))
    assert(0);
  assert(layer->source);
  assert(layer->type == SIMPLET_RASTER);
  simplet_raster_layer_free(layer);
}

static void test_user_data() {
  simplet_raster_layer_t *layer;
  if (!(layer = simplet_raster_layer_new("./data/loss_1932_2010.tif")))
    assert(0);
  int i = 1;
  simplet_raster_layer_set_user_data(layer, &i);
  int *j;
  j = simplet_raster_layer_get_user_data(layer);
  assert(i == *j);
  simplet_raster_layer_free(layer);
}

TASK(raster_layer) {
  test(raster_layer);
  test(user_data);
}