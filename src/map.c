#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "style.h"



static void
rule_free(void *rule){
  simplet_rule_t *tmp = rule;
  simplet_list_free(tmp->styles);
  free(tmp->ogrsql);
  free(tmp);
}

static char*
copy_string(char *src){
  int len = strlen(src);
  char *dest;
  if(!(dest = malloc(len + 1)))
    return NULL;
  memcpy(src, dest, len);
  dest[len] = '\0';
  return dest;
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
  map->valid  = MAP_OK;
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
  if(map->rules)
    simplet_list_free(map->rules);
  free(map);
}

int
simplet_map_set_srs(simplet_map_t *map, char *proj){
  if(!(map->proj = OSRNewSpatialReference(proj)))
    return (map->valid = MAP_ERR);
  return MAP_OK;
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
  if(!(bounds = simplet_bounds_new(bounds)))
    return (map->valid = MAP_ERR);
  simplet_bounds_extend(bounds, maxx, maxy);
  simplet_bounds_extend(bounds, minx, miny);
  return MAP_OK;
}

int
simplet_map_layer(simplet_map_t *map, char *datastring){
  if(!(map->source = OGROpen(datastring, 0, NULL)))
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
  
  char *sql;
  if(!(sql = copy_string(sqlquery)))
    return (map->valid = MAP_ERR);

  rule->ogrsql = sql;
  
  if(!simplet_list_push(map->rules, rule))
    return (map->valid = MAP_ERR);
  
  return MAP_OK;
}

int
simplet_map_add_style(simplet_map_t *map, char *key, char *arg){
  if(!map->rules->tail)
    return (map->valid = MAP_ERR);
  simplet_rule_t *rule = map->rules->tail->value;
  
  simplet_style_t *style;
  if(!(style = malloc(sizeof(*style))))
    return (map->valid = MAP_ERR);
  
  for(int i = 0; i < SIMPLET_STYLES_LENGTH; i++){
    if(strcmp(key, styleTable[i].key)){
      style->call = styleTable[i].call;
      style->key  = copy_string(styleTable[i].key);
      style->arg  = copy_string(arg);
    }
  }
  
  if(!(style->arg || style->call))
    return (map->valid = MAP_ERR);
  
  simplet_list_push(rule->styles, style);
  return MAP_OK;
}

int
simplet_map_render_to_png(simplet_map_t *map, char *path){
  
}