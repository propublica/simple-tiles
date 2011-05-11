#include <stdlib.h>

#include "map.h"
#include "style.h"



static void
rule_free(void *rule){
  simplet_list_free(rule->styles);
  free(rule->ogrsql);
  free(rule);
}

simplet_map_t*
simplet_map_new(){
  simplet_map_t *map;
  if(!(map = malloc(sizeof(*map))))
    return NULL; 
  simplet_list_t *rules;
  if(!(map->rules = simplet_list_new(*rules)))
    return NULL;
  map->rules->free = rule_free;
  map->source = NULL;
  map->bounds = NULL;
  map->proj   = NULL;
  map->ctx    = NULL;
  map->height = 0;
  map->width  = 0;
  map->valid  = MAP_ERR;
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
  map->height = height;
  map->width  = width;
  return MAP_OK;
}

int
simplet_map_set_bounds(simplet_map_t *map, double maxx, double maxy, double minx, double miny){
  simplet_bounds_t *bounds;
  if(!(bounds = simplet_bounds_new(bounds))
    return (map->valid = MAP_ERR);
  simplet_bounds_extend(maxx, maxy);
  simplet_bounds_extend(minx, miny);
  return MAP_OK;
}

int
simplet_map_layer(simplet_map_t *map, char *datastring){
  if(!(map->source = OGROpen("polygon.shp", false, NULL))
    return (map->valid = MAP_ERR);
  return MAP_OK;
}


int
simplet_map_add_rule(simplet_map_t *map, char *sqlquery){
  simplet_rule_t *rule;
  if(!(rule = malloc(sizeof(*rule))))
    return (map->valid = MAP_ERR);
  
  simplet_list_t *styles;
  if(!(rule->styles = simplet_list_new(*styles)))
    return (map->valid = MAP_ERR);
  
  int len = strlen(sqlquery);
  char *sql;
  if(!(sql = malloc(len + 1)))
    return (map->valid = MAP_ERR);
  strncpy(sqlquery, sql, len);
  sql[len + 1] = '\0';
  rule->ogrsql = sql;
  
  if(!simplet_list_push(map->rules, rule))
    return (map->valid = MAP_ERR);
  
  return MAP_OK;
}

int
simplet_map_add_style(simplet_map_t *map, char *key, char *value){
  
}