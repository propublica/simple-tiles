#ifndef __SIMPLE_TILES_MAP_H__
#define __SIMPLE_TILES_MAP_H__

#include <gdal/ogr_api.h>
#include <gdal/ogr_srs_api.h>
#include <cairo/cairo.h>
#include "list.h"
#include "bounds.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char *ogrsql;
  simplet_list_t *styles;
} simplet_rule_t;

typedef struct simplet_map_t {
  OGRDataSourceH       *source;
  simplet_bounds_t     *bounds;
  simplet_list_t       *rules;
  OGRSpatialReferenceH *proj;
  cairo_t              *ctx;
} simplet_map_t;

simplet_map_t*
simplet_map_new();

void
simplet_map_free(simplet_map_t *map);

int
simplet_map_set_srs(simplet_map_t *map, char *proj);

int
simplet_map_size(simplet_map_t *map, int width, int height);

int
simplet_map_zoom(simplet_map_t *map, double maxx, double maxy, double minx, double miny);

int
simplet_map_layer(simplet_map_t *map, char *datastring);

int
simplet_map_add_rule(simplet_map_t *map, char *sqlquery);

int
simplet_map_add_style(simplet_map_t *map, char *key, char *value);

int
simplet_map_isvalid(simplet_map_t *map);

int
simplet_map_render(char *string);

#ifdef __cplusplus
}
#endif

#endif