#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <simple-tiles/simple_tiles.h>
#include <simple-tiles/map.h>

int
main(){
  simplet_map_t *map;
  if(!(map = simplet_map_new()))
    exit(1);

  simplet_map_set_slippy(map, 0, 0, 0);
  simplet_map_set_size(map, 1000, 1000);

  simplet_map_set_bgcolor(map, "#ddeeff");

  simplet_map_add_layer(map, "../data/ne_10m_admin_0_countries.shp");

  simplet_map_add_filter(map,  "SELECT * from 'ne_10m_admin_0_countries'");
  simplet_map_add_style(map, "stroke", "#226688");
  simplet_map_add_style(map, "line-join", "round");
  simplet_map_add_style(map, "weight", "3");

  simplet_map_add_filter(map, "SELECT * from 'ne_10m_admin_0_countries'");
  simplet_map_add_style(map, "weight", "0.5");
  simplet_map_add_style(map, "fill", "#d3e46f");
  simplet_map_add_style(map, "stroke", "#ffffff");
  simplet_map_add_style(map, "line-join", "round");

  simplet_map_add_style(map, "text-field", "NAME");
  simplet_map_add_style(map, "font", "Lucida Grande, Regular 12");
  simplet_map_add_style(map, "color", "#444444ff");
  simplet_map_add_style(map, "text-halo-color", "#ffffffcc");
  simplet_map_add_style(map, "text-halo-weight", "2");
  simplet_map_add_style(map, "letter-spacing", "1");
  if(simplet_map_is_valid(map))
    simplet_map_render_to_png(map, "./out.png");
  simplet_map_free(map);
}
