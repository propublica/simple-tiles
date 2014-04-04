#ifndef _SIMPLE_TILES_MAP_H
#define _SIMPLE_TILES_MAP_H

#include "types.h"
#include "user_data.h"

#ifdef __cplusplus
extern "C" {
#endif

simplet_map_t*
simplet_map_new();

void
simplet_map_free(simplet_map_t *map);

simplet_status_t
simplet_map_set_srs(simplet_map_t *map, const char *proj);

simplet_status_t
simplet_map_set_size(simplet_map_t *map, unsigned int width, unsigned int height);

simplet_status_t
simplet_map_set_bounds(simplet_map_t *map, double maxx, double maxy, double minx, double miny);

simplet_status_t
simplet_map_set_bgcolor(simplet_map_t *map, const char *str);

void
simplet_map_get_bgcolor(simplet_map_t *map, char **str);

simplet_vector_layer_t*
simplet_map_add_vector_layer(simplet_map_t *map, const char *datastring);

simplet_vector_layer_t*
simplet_map_add_vector_layer_directly(simplet_map_t *map, simplet_layer_t *layer);

simplet_status_t
simplet_map_get_status(simplet_map_t *map);

const char*
simplet_map_status_to_string(simplet_map_t *map);

simplet_status_t
simplet_map_is_valid(simplet_map_t *map);

void
simplet_map_render_to_png(simplet_map_t *map, const char *path);

void
simplet_map_render_to_stream(simplet_map_t *map, void *stream,
  cairo_status_t (*cb)(void *closure, const unsigned char *data, unsigned int length));

void
simplet_map_get_srs(simplet_map_t *map, char **srs);

simplet_status_t
simplet_map_set_slippy(simplet_map_t *map, unsigned int x, unsigned int y, unsigned int z);

void
simplet_map_init_matrix(simplet_map_t *map, cairo_matrix_t *mat);

double
simplet_map_get_buffer(simplet_map_t *map);

void
simplet_map_set_buffer(simplet_map_t *map, double buffer);

unsigned int
simplet_map_get_width(simplet_map_t *map);

unsigned int
simplet_map_get_height(simplet_map_t *map);

simplet_status_t
simplet_map_set_width(simplet_map_t *map, unsigned int width);

simplet_status_t
simplet_map_set_height(simplet_map_t *map, unsigned int height);

SIMPLET_HAS_USER_DATA_PROTOS(map)

#ifdef __cplusplus
}
#endif

#endif
