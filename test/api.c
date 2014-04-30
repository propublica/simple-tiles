#include <stdlib.h>
#include <assert.h>
#include <cpl_error.h>
#include <cpl_conv.h>
#include "simple_tiles.h"
#include "query.h"
#include "vector_layer.h"

int
main(){
  CPLSetConfigOption("CPL_DEBUG", "ON");

  simplet_map_t *map;
  if(!(map = simplet_map_new()))
    exit(1);

  simplet_map_set_slippy(map, 0, 0, 0);
  simplet_map_set_size(map, 1000, 1000);
  simplet_map_set_bgcolor(map, "#ddeeff");

  simplet_vector_layer_t *layer = simplet_map_add_vector_layer(map, "./data/ne_10m_admin_0_countries.shp");
  simplet_query_t *query = simplet_vector_layer_add_query(layer, "SELECT * from 'ne_10m_admin_0_countries'");
  simplet_query_add_style(query, "stroke", "#226688");
  simplet_query_add_style(query, "line-join", "round");
  simplet_query_add_style(query, "weight", "3");

  simplet_query_t *query2 = simplet_vector_layer_add_query(layer, "SELECT * from 'ne_10m_admin_0_countries'");
  simplet_query_add_style(query, "blend", "over");

  simplet_query_add_style(query2, "weight", "0.5");
  simplet_query_add_style(query2, "fill", "#d3e46f");
  simplet_query_add_style(query2, "stroke", "#ffffff");
  simplet_query_add_style(query2, "line-join", "round");

  simplet_query_add_style(query2, "text-field", "NAME");
  simplet_query_add_style(query2, "font", "Lucida Grande, Regular 8");
  simplet_query_add_style(query2, "color", "#444444ff");
  simplet_query_add_style(query2, "text-stroke-color", "#ffffffcc");
  simplet_query_add_style(query2, "text-stroke-weight", "2");
  simplet_query_add_style(query2, "letter-spacing", "1");


  if(simplet_map_is_valid(map))
    simplet_map_render_to_png(map, "./out.png");

  simplet_map_free(map);
}
