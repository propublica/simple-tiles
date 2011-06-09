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


  simplet_map_set_srs(map, "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs +over");
  simplet_map_set_size(map, 256, 256);
  simplet_map_set_bounds(map, -19951913.227845426648855, 2017836.357428819872439,-5661673.7908283341676, 11554793.571191838011146);

  simplet_map_add_layer(map, "../data/tl_2010_us_state10.shp");
  simplet_map_add_filter(map,  "SELECT * from tl_2010_us_state10");
  simplet_map_add_style(map, "weight", "0.3");
  simplet_map_add_style(map, "fill",   "#061F37ff");
  simplet_map_add_style(map, "stroke", "#ffffff");
  clock_t start = clock();
  if(simplet_map_is_valid(map))
    simplet_map_render_to_png(map, "./out.png");
  printf("render time: %f", (float) (clock() - start) / CLOCKS_PER_SEC);
  simplet_map_free(map);
}
