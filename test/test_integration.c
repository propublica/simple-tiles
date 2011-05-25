#include "test.h"
#include <simple-tiles/map.h>

simplet_map_t*
build_map(){
  simplet_map_t *map;
  if(!(map = simplet_map_new()))
    exit(1);

  simplet_map_set_srs(map, "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs");
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
  return map;
}

cairo_bool_t
point_in_fill(simplet_map_t *map, double x, double y){
  OGRGeometryH point;
  assert((point = OGR_G_CreateGeometry(wkbPoint)));
  OGR_G_AddPoint_2D(point, x, y);
  
  
  cairo_user_to_device(map->_ctx, &x, &y);
  return cairo_in_fill(map->_ctx, x, y);
}




void
test_many_layers(){

}
void
test_many_rules(){
  
}


void
test_plot(){

}

void
test_points(){
  simplet_map_t *map;
  assert((map = build_map()));
  cairo_surface_t* surface;
  assert((surface = simplet_map_build_surface(map)));
  /* Lake Winnebago */
  assert(!point_in_fill(map, 44.0280815, -88.4210006));
  //assert(point_in_fill(map, ))
  simplet_map_close_surface(map, surface);
  simplet_map_free(map);
}

TASK(integration){
  test(plot);
  test(points);
  test(many_rules);
  test(many_layers);
}