#include "test.h"
#include <simple-tiles/map.h>

void
test_map(){
  simplet_map_t *map;
  if(!(map = simplet_map_new()))
    assert(0);
  simplet_map_set_srs(map, "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs");
  assert(map->proj);
  simplet_map_set_size(map, 256, 256);
  assert(map->width == 256);
  assert(map->height == 256);
  simplet_map_set_bounds(map, -179.231086, 17.831509, -100.859681, 71.441059);
  assert(map->bounds);
  simplet_map_free(map);
}

void
test_creation(){
  simplet_map_t *map;
  if(!(map = simplet_map_new()))
    assert(0);
  simplet_layer_t *layer = simplet_map_add_layer(map, "../data/tl_2010_us_cd108.shp");
  assert(map->layers->length == 1);
  assert(map->layers->head->value == layer);
  simplet_rule_t *rule = simplet_map_add_rule(map,  "SELECT * from tl_2010_us_cd108");
  assert(rule == layer->rules->head->value);
  assert(layer->rules->length == 1);
  simplet_style_t *style = simplet_map_add_style(map, "line-cap",  "square");
  assert(rule->styles->length == 1);
  assert(style == rule->styles->head->value);
  simplet_map_free(map);
}

TASK(map){
  test(map);
  test(creation);
}