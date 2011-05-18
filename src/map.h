#ifndef _SIMPLE_TILES_MAP_H
#define _SIMPLE_TILES_MAP_H

#include "types.h"
#include "list.h"
#include "bounds.h"
#include "style.h"


#define MAP_OK 1
#define MAP_ERR 0

#ifdef __cplusplus
extern "C" {
#endif

simplet_map_t*
simplet_map_new();

void
simplet_map_free(simplet_map_t *map);

int
simplet_map_set_srs(simplet_map_t *map, char *proj);

int
simplet_map_set_size(simplet_map_t *map, int width, int height);

int
simplet_map_set_bounds(simplet_map_t *map, double maxx, double maxy, double minx, double miny);

int
simplet_map_add_layer(simplet_map_t *map, char *datastring);

simplet_rule_t*
simplet_map_add_rule(simplet_map_t *map, char *sqlquery);

simplet_style_t*
simplet_map_add_style(simplet_map_t *map, char *key, char *arg);

int
simplet_map_isvalid(simplet_map_t *map);

int
simplet_map_render_to_png(simplet_map_t *map, char *path);

#ifdef __cplusplus
}
#endif

#endif