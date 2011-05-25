#include "test.h"
#include <simple-tiles/map.h>

simplet_map_t*
build_map(){
  simplet_map_t *map;
  if(!(map = simplet_map_new()))
    exit(1);

  simplet_map_set_srs(map, "+proj=longlat +ellps=GRS80 +datum=NAD83 +no_defs");
  simplet_map_set_size(map, 256, 256);
  simplet_map_set_bounds(map, -179.231086, 17.831509, -100.859681, 71.441059);
  // Only one layer per map for now

  simplet_map_add_layer(map, "../data/tl_2010_us_cd108.shp");
  simplet_map_add_rule(map,  "SELECT * from tl_2010_us_cd108");
  simplet_map_add_style(map, "line-cap",  "square");
  simplet_map_add_style(map, "line-join", "round");
  simplet_map_add_style(map, "fill",      "#061F37ff");
  //simplet_map_add_style(map, "weight",    "0.03");
  //simplet_map_add_style(map, "stroke",    "#666666ff");
  return map;
}



void
test_many_layers(){

}
void
test_many_rules(){

}


void
test_projection(){
  simplet_map_t *map;
  assert((map = build_map()));
  simplet_map_set_srs(map, "+proj=aea +lat_1=27.5 +lat_2=35 +lat_0=18 +lon_0=-100 +x_0=1500000 +y_0=6000000 +ellps=GRS80 +datum=NAD83 +units=m +no_defs");
  simplet_map_add_style(map, "seemless", "true");
  assert(simplet_map_isvalid(map));
  simplet_map_render_to_png(map, "./projection.png");
  simplet_map_free(map);
}

TASK(integration){
  test(projection);
  puts("check projection.png");
  test(many_rules);
  test(many_layers);
}