#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <simple-tiles/simple_tiles.h>

int
main(){
  simplet_map_t *map;
  if(!(map = simplet_map_new()))
    exit(1);

  simplet_map_set_size(map, 256, 256);
  simplet_map_set_slippy(map, 0, 0, 0);
  simplet_map_add_layer(map, "../data/10m_admin_0_countries.shp");
  simplet_map_add_filter(map,  "SELECT * from '10m_admin_0_countries'");
  simplet_map_add_style(map, "weight", "0.1");
  simplet_map_add_style(map, "fill",   "#061F3799");
  simplet_map_add_style(map, "seamless", "true");
  if(simplet_map_is_valid(map))
    simplet_map_render_to_png(map, "./out.png");
  simplet_map_free(map);
}
