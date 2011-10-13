#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <simple-tiles/simple_tiles.h>
#include <simple-tiles/filter.h>
#include <simple-tiles/layer.h>
#define CHECK(x) if(x == NULL) { puts("##x is null!"); exit(1); }
int
main(){
  simplet_map_t *map;
  if(!(map = simplet_map_new()))
    exit(1);

  simplet_map_set_size(map, 256, 256);
  simplet_map_set_slippy(map, 0, 0, 0);
  simplet_layer_t *layer = simplet_map_add_layer(map, "../data/ne_10m_admin_0_countries.shp");
  CHECK(layer)
  simplet_filter_t *filter = simplet_layer_add_filter(layer,  "SELECT * from 'ne_10m_admin_0_countries'");
  CHECK(filter)
  simplet_style_t *style   = simplet_filter_add_style(filter, "weight", "0.1");
  CHECK(style);
  simplet_style_t *style1  = simplet_filter_add_style(filter, "fill", "#061F3799");
  CHECK(style1);
  simplet_style_t *style2  = simplet_filter_add_style(filter, "stroke", "#ffffff99");
  CHECK(style2);
  if(simplet_map_is_valid(map))
    simplet_map_render_to_png(map, "./out.png");
  simplet_map_free(map);
}
