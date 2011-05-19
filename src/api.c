#include <stdlib.h>
#include "simple_tiles.h"

int
main(){
  simplet_map_t *map;
  if(!(map = simplet_map_new()))
    exit(1);

  simplet_map_set_srs(map, "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs");
  simplet_map_set_size(map, 600, 300);
  simplet_map_set_bounds(map, -179.231086, 17.831509, -60.859681, 71.441059);
  // Only one layer per map for now

  simplet_map_add_layer(map, "../data/tl_2010_us_cd108.shp");
  simplet_map_add_rule(map,  "SELECT * from tl_2010_us_cd108");
  simplet_map_add_style(map, "line-cap",  "square");
  simplet_map_add_style(map, "line-join", "round");
  simplet_map_add_style(map, "fill",      "#4c4c3f");
  simplet_map_add_style(map, "weight",    "0.01");
  simplet_map_add_style(map, "stroke",    "#a8a96dff");
  
  simplet_map_add_layer(map, "../data/armory2.shp");
  simplet_map_add_rule(map,  "SELECT * from armory2");
  simplet_map_add_style(map, "radius",    "4");
  simplet_map_add_style(map, "fill",      "#cc000099");
  simplet_map_add_style(map, "weight",    "0.1");
  simplet_map_add_style(map, "stroke",    "#a8a96dff");

  if(simplet_map_isvalid(map))
    simplet_map_render_to_png(map, "./out.png");
  simplet_map_free(map);
}