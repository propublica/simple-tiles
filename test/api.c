#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <simple-tiles/simple_tiles.h>
#include <simple-tiles/filter.h>
#include <simple-tiles/layer.h>
int
main(){
  simplet_map_t *map;
  if(!(map = simplet_map_new()))
    exit(1);

  simplet_map_set_slippy(map, 0, 0, 0);
  simplet_map_set_size(map, 1000, 1000);
  simplet_map_set_bgcolor(map, "#ddeeff");

  simplet_layer_t *layer   = simplet_map_add_layer(map, "../data/ne_10m_admin_0_countries.shp");
  simplet_filter_t *filter = simplet_layer_add_filter(layer,  "SELECT * from 'ne_10m_admin_0_countries'");
  simplet_filter_add_style(filter, "stroke", "#226688");
  simplet_filter_add_style(filter, "line-join", "round");
  simplet_filter_add_style(filter, "weight", "3");

  simplet_filter_t *filter2 = simplet_layer_add_filter(layer, "SELECT * from 'ne_10m_admin_0_countries'");
  simplet_filter_add_style(filter2, "weight", "0.5");
  simplet_filter_add_style(filter2, "fill", "#d3e46f");
  simplet_filter_add_style(filter2, "stroke", "#ffffff");
  simplet_filter_add_style(filter2, "line-join", "round");

  simplet_filter_add_style(filter2, "text-field", "NAME");
  simplet_filter_add_style(filter2, "font", "Lucida Grande, Regular 12");
  simplet_filter_add_style(filter2, "color", "#444444ff");
  simplet_filter_add_style(filter2, "text-halo-color", "#ffffffcc");
  simplet_filter_add_style(filter2, "text-halo-weight", "2");
  simplet_filter_add_style(filter2, "letter-spacing", "1");

  if(simplet_map_is_valid(map))
    simplet_map_render_to_png(map, "./out.png");
  simplet_map_free(map);
}
