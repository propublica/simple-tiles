#include <stdlib.h>
#include <simple-tiles/simple_tiles.h>

int
main(){
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
  simplet_map_add_style(map, "fill",      "#ffffffff");
  simplet_map_add_style(map, "weight",    "0.03");
  simplet_map_add_style(map, "stroke",    "#666666ff");


  if(simplet_map_isvalid(map))
    simplet_map_render_to_png(map, "./out.png");
  simplet_map_free(map);
}