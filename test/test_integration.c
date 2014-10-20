#include "map.h"
#include "vector_layer.h"
#include "raster_layer.h"
#include "query.h"
#include "list.h"
#include "test.h"

simplet_map_t*
build_map(){
  simplet_map_t *map;
  assert((map = simplet_map_new()));
  simplet_map_set_srs(map, "+proj=longlat +ellps=GRS80 +datum=NAD83 +no_defs");
  simplet_map_set_size(map, 256, 256);
  simplet_map_set_bounds(map,
      -179.231086, 17.831509, -100.859681, 71.441059);
  simplet_vector_layer_t  *layer  = simplet_map_add_vector_layer(map,
      "./data/ne_10m_admin_0_countries.shp");
  simplet_query_t *query = simplet_vector_layer_add_query(layer,
      "SELECT * from 'ne_10m_admin_0_countries'");
  simplet_query_add_style(query, "line-cap",  "square");
  simplet_query_add_style(query, "line-join", "round");
  simplet_query_add_style(query, "fill",      "#061F3799");
  simplet_query_add_style(query, "seamless",  "true");
  return map;
}

void
test_background(){
  simplet_map_t *map;
  assert((map = build_map()));
  simplet_map_set_bgcolor(map, "#CC0000");
  simplet_map_render_to_png(map, "./background.png");
  assert(SIMPLET_OK == simplet_map_get_status(map));
  simplet_map_free(map);
}

void
run_test_raster(bool resample, char* filename){
  simplet_map_t *map;
  assert((map = simplet_map_new()));
  simplet_map_set_srs(map, "EPSG:3857");
  simplet_map_set_bounds(map, -8317853.988, 5070182.233, -8011088.731, 4757312.390);
  simplet_map_set_size(map, 400, 400);
  simplet_map_set_slippy(map, 4822,6159,14);
  simplet_raster_layer_t *layer = simplet_map_add_raster_layer(map, "./data/nyc2-rgb-pansharpened-8bit-nodata.tif");
  layer->resample = resample;
  assert(simplet_map_is_valid(map));
  simplet_map_render_to_png(map, filename);
  assert(SIMPLET_OK == simplet_map_get_status(map));
  simplet_map_free(map);
}

void
test_raster() {
  run_test_raster(false, "./raster.png");
}

void
test_raster_resample() {
  run_test_raster(true, "./raster-resample.png");
}

void
test_many_layers(){
  simplet_map_t *map;
  assert((map = build_map()));
  assert(simplet_map_is_valid(map));
  simplet_vector_layer_t  *layer  = simplet_map_add_vector_layer(map,
      "./data/ne_10m_admin_0_countries.shp");
  simplet_query_t *query = simplet_vector_layer_add_query(layer,
      "SELECT * from 'ne_10m_admin_0_countries' where SOV_A3 = 'US1'");
  simplet_query_add_style(query, "fill", "#cc0000dd");
  simplet_map_render_to_png(map, "./layers.png");
  assert(SIMPLET_OK == simplet_map_get_status(map));
  simplet_map_free(map);
}

void
test_many_queries(){
  simplet_map_t *map;
  assert((map = build_map()));
  assert(simplet_map_is_valid(map));
  simplet_query_t *query = simplet_vector_layer_add_query(
      (simplet_vector_layer_t *) simplet_list_get(map->layers, simplet_list_get_length(map->layers) - 1),
      "SELECT * from 'ne_10m_admin_0_countries' where SOV_A3 = 'US1'");

  simplet_query_add_style(query, "weight", "1");
  simplet_query_add_style(query, "stroke", "#00cc00dd");
  simplet_query_add_style(query, "fill",   "#cc0000");
  simplet_query_add_style(query, "blend",  "multiply");

  simplet_map_render_to_png(map, "./queries.png");
  assert(SIMPLET_OK == simplet_map_get_status(map));

  simplet_map_free(map);
}

void
test_projection(){
  simplet_map_t *map;
  assert((map = build_map()));
  simplet_map_set_srs(map, "+proj=aea +lat_1=27.5 +lat_2=35 +lat_0=18 +lon_0=-100 +x_0=1500000 +y_0=6000000 +ellps=GRS80 +datum=NAD83 +units=m +no_defs +over");
  simplet_map_set_bounds(map, -3410023.644683, 12407191.9541633, 5198986.57554026, 6500142.362205);
  assert(simplet_map_is_valid(map));
  simplet_map_render_to_png(map, "./projection.png");
  assert(SIMPLET_OK == simplet_map_get_status(map));
  simplet_map_free(map);
}

void
test_slippy_gen(){
  simplet_map_t *map;
  assert((map = build_map()));
  simplet_map_set_slippy(map, 0, 0, 1);
  simplet_map_render_to_png(map, "./slippy.png");
  assert(SIMPLET_OK == simplet_map_get_status(map));
  simplet_map_free(map);
}

void
test_holes(){
  simplet_map_t *map;
  assert((map = simplet_map_new()));
  simplet_map_set_srs(map, "+proj=longlat +ellps=GRS80 +datum=NAD83 +no_defs");
  simplet_map_set_size(map, 256, 256);
  simplet_map_set_bounds(map, -92.889433, 42.491912,-86.763988, 47.080772);
  simplet_map_add_vector_layer(map, "./data/tl_2010_55_cd108.shp");
  simplet_query_t *query = simplet_vector_layer_add_query(
      (simplet_vector_layer_t *) simplet_list_tail(map->layers),
      "SELECT * from 'tl_2010_55_cd108'");
  simplet_query_add_style(query, "line-cap",  "square");
  simplet_query_add_style(query, "line-join", "round");
  simplet_query_add_style(query, "fill",      "#061F3799");
  simplet_query_add_style(query, "stroke",    "#ffffff99");
  simplet_query_add_style(query, "weight",    "0.1");
  simplet_map_render_to_png(map, "./holes.png");
  assert(SIMPLET_OK == simplet_map_get_status(map));
  simplet_map_free(map);
}

void
test_points(){
	simplet_map_t *map;
  assert((map = simplet_map_new()));
	simplet_map_set_srs(map, "+proj=longlat +ellps=GRS80 +datum=NAD83 +no_defs");
  simplet_map_set_size(map, 256, 256);
  simplet_map_set_bounds(map, -92.889433, 42.491912,-86.763988, 47.080772);
  simplet_vector_layer_t  *layer  = simplet_map_add_vector_layer(map, "./data/ne_10m_populated_places.shp");
  simplet_query_t *query = simplet_vector_layer_add_query(layer,  "SELECT * from 'ne_10m_populated_places'");
	simplet_query_add_style(query, "fill",   "#061F3799");
  simplet_query_add_style(query, "stroke", "#ffffff99");
  simplet_query_add_style(query, "weight", "0.1");
  simplet_query_add_style(query, "radius", "10");
	simplet_map_render_to_png(map, "./points.png");
  assert(SIMPLET_OK == simplet_map_get_status(map));
  simplet_map_free(map);
}

void
test_lines(){
  simplet_map_t *map;
  assert((map = simplet_map_new()));
  simplet_map_set_srs(map, "+proj=longlat +ellps=GRS80 +datum=NAD83 +no_defs");
  simplet_map_set_size(map, 256, 256);
  simplet_map_set_bounds(map, -74.043825, 40.570771, -73.855660, 40.739255);
  simplet_vector_layer_t  *layer  = simplet_map_add_vector_layer(map, "./data/tl_2010_36047_roads.shp");
  simplet_query_t *query = simplet_vector_layer_add_query(layer,  "SELECT * from 'tl_2010_36047_roads'");
  simplet_query_add_style(query, "stroke",    "#000000ff");
	simplet_query_add_style(query, "line-cap",  "square");
  simplet_query_add_style(query, "line-join", "round");
  simplet_query_add_style(query, "weight",    "0.3");
  simplet_map_render_to_png(map, "./lines.png");
  assert(SIMPLET_OK == simplet_map_get_status(map));
  simplet_map_free(map);
}

void
test_bunk(){
  simplet_map_t *map;
  assert((map = simplet_map_new()));
  simplet_map_set_srs(map, "+proj=longlat +ellps=GRS80 +datum=NAD83 +no_defs");
  simplet_map_set_size(map, 256, 256);
  simplet_map_set_bounds(map, -74.043825, 40.570771, -73.855660, 40.739255);
  simplet_vector_layer_t  *layer  = simplet_map_add_vector_layer(map, "./data/tl_2010_36047_roads.shp");
  simplet_vector_layer_add_query(layer, "SELECT * from 'tl_2010_36047_roads_bunk'");
  simplet_map_render_to_png(map, "./bunk.png");
  assert(SIMPLET_OK != simplet_map_get_status(map));
  printf("ERROR AS EXPECTED: %s", simplet_map_status_to_string(map));
  simplet_map_free(map);
}

cairo_status_t
stream(void *closure, const unsigned char *data, unsigned int length){
  data = NULL, length = 0, closure = NULL; /* suppress warnings */
  return CAIRO_STATUS_SUCCESS;
}

void
test_stream(){
  simplet_map_t *map;
  assert((map = build_map()));
  char *data = NULL;
  simplet_map_render_to_stream(map, data, stream);
  assert(SIMPLET_OK == simplet_map_get_status(map));
  simplet_map_free(map);
}

TASK(integration){
  // test(projection);
  // puts("check projection.png");
  // test(many_queries);
  // puts("check queries.png");
  // test(many_layers);
  // puts("check layers.png");
  test(raster);
  puts("check raster.png");
  test(raster_resample);
  puts("check raster-resample.png");
  // test(slippy_gen);
  // puts("check slippy.png");
  // test(stream);
  // puts("check holes.png");
  // test(holes);
  // puts("check lines.png");
  // test(lines);
  // puts("check points.png");
  // test(points);
  // puts("check background.png");
  // test(background);
  // test(bunk);
}
