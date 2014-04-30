#include "test.h"
#include "list.h"
#include "vector_layer.h"

void
test_vector_layer(){
  simplet_vector_layer_t *layer;
  if(!(layer = simplet_vector_layer_new("../data/tl_2010_us_cd108.shp")))
    assert(0);
  assert(layer->source);
  simplet_vector_layer_free(layer);
}

void
test_add_query(){
  simplet_vector_layer_t *layer;
  if(!(layer = simplet_vector_layer_new("../data/tl_2010_us_cd108.shp")))
    assert(0);
  simplet_vector_layer_add_query(layer, "SELECT * from tl_2010_us_cd108");
  simplet_vector_layer_add_query(layer, "SELECT * from tl_2010_us_cd108 where STATEFP00 = '47'");
  simplet_vector_layer_add_query(layer, "SELECT * from tl_2010_us_cd108 where STATEFP00 = '47'");
  assert(simplet_list_get_length(layer->queries) == 3);
  simplet_vector_layer_free(layer);
}

TASK(vector_layer){
  test(vector_layer);
  test(add_query);
}
