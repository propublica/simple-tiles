#ifndef _SIMPLE_TILES_MAP_H
#define _SIMPLE_TILES_MAP_H

#include "types.h"
#include "list.h"
#include "bounds.h"
#include "style.h"
#include "layer.h"


#ifdef __cplusplus
extern "C" {
#endif

simplet_map_t*
simplet_map_new();

void
simplet_map_free(simplet_map_t *map);

int
simplet_map_set_srs(simplet_map_t *map, const char *proj);

int
simplet_map_set_size(simplet_map_t *map, int width, int height);

int
simplet_map_set_bounds(simplet_map_t *map, double maxx, double maxy, double minx, double miny);

simplet_layer_t*
simplet_map_add_layer(simplet_map_t *map, const char *datastring);

simplet_filter_t*
simplet_map_add_filter(simplet_map_t *map, const char *sqlquery);

simplet_style_t*
simplet_map_add_style(simplet_map_t *map, const char *key, const char *arg);

simplet_status_t
simplet_map_is_valid(simplet_map_t *map);

int
simplet_map_render_to_png(simplet_map_t *map, const char *path);

int
simplet_map_render_to_stream(simplet_map_t *map, void *stream,
  cairo_status_t (*cb)(void *closure, const unsigned char *data, unsigned int length));

cairo_surface_t *
simplet_map_build_surface(simplet_map_t *map);

void
simplet_map_close_surface(simplet_map_t *map, cairo_surface_t *surface);

void
simplet_map_scale(simplet_map_t *map);

void
simplet_map_get_srs(simplet_map_t *map, char **srs);

int
simplet_map_set_slippy(simplet_map_t *map, unsigned int x, unsigned int y, unsigned int z);

#ifdef __cplusplus
}
#endif

#endif