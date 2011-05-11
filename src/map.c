#include <stdlib.h>

#include "map.h"
#include "style.h"


simplet_map_t*
simplet_map_new(){
  simplet_map_t* map;
  if(!(map = malloc(sizeof(*map))))
    return NULL;
  map->source = NULL;
  map->bounds = NULL;
  map->rules  = NULL;
  map->proj   = NULL;
  map->ctx    = NULL;
  return map;
}

void
simplet_map_free(simplet_map_t *map){
  if(map->bounds)
    simplet_bounds_free(map->bounds);
  if(map->source)
    OGR_DS_Destroy(map->source);
  if(map->proj)
    OSRRelease(map->proj);
  if(map->ctx)
    cairo_destroy(map->ctx);
  if(map->rules)
    simplet_list_free(map->rules);
  free(map);
}

int
simplet_map_set_srs(simplet_map_t *map, char *proj){
  
}

int
simplet_map_size(simplet_map_t *map, int width, int height){
  
}

int
simplet_map_zoom(simplet_map_t *map, double maxx, double maxy, double minx, double miny){
  
}

int
simplet_map_layer(simplet_map_t *map, char *datastring){
  
}

int
simplet_map_add_rule(simplet_map_t *map, char *sqlquery){
  
}

int
simplet_map_add_style(simplet_map_t *map, char *key, char *value){
  
}