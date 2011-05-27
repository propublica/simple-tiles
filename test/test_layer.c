#include "test.h"
#include <simple-tiles/layer.h>

void
test_layer(){
  simplet_layer_t *layer;
  if(!(layer = simplet_layer_new("../data/tl_2010_us_cd108.shp")))
    assert(0);
  assert(layer->source);
  simplet_layer_free(layer);
}

void
test_add_rule(){
  simplet_layer_t *layer;
  if(!(layer = simplet_layer_new("../data/tl_2010_us_cd108.shp")))
    assert(0);
  simplet_layer_add_rule(layer, "SELECT * from tl_2010_us_cd108");
  simplet_layer_add_rule(layer, "SELECT * from tl_2010_us_cd108 where STATEFP00 = '47'");
  simplet_layer_add_rule(layer, "SELECT * from tl_2010_us_cd108 where STATEFP00 = '47'");
  assert(layer->rules->length == 3);
  simplet_layer_free(layer);
}

TASK(layer){
  test(layer);
  test(add_rule);
}