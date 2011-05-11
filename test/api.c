#include <simple_tiles.h>

int
main(){
  simplet_map_t *map;

  if(simplet_map_new(map) == NULL)
    exit(1);

  simplet_map_set_srs(map, "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs");
  simplet_map_size(map, 256, 256);
  simplet_map_zoom(map, 1000, 1000, 1000, 1000);
  simplet_map_layer(map, "./test.shp");
  simplet_map_add_rule(map,  "SELECT * from test where TEST < 5");
  simplet_map_add_style(map, "fill",   "#cc0000");
  simplet_map_add_style(map, "stroke", "#aaaaaa");
  simplet_map_add_rule(map,  "SELECT * from test where TEST > 5");
  simplet_map_add_style(map, "fill",   "#aa0000");
  simplet_map_add_style(map, "stroke", "#444444");
  simplet_map_add_rule(map,  "SELECT * from test");


  if(simplet_map_is_valid(map))
    simplet_map_render("./out.png");
  
  simplet_map_free(map);
}