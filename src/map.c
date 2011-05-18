#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "map.h"
#include "style.h"
#include "rule.h"
#include "util.h"

simplet_map_t*
simplet_map_new(){
  simplet_map_t *map;
  if(!(map = malloc(sizeof(*map))))
    return NULL;

  if(!(map->rules = simplet_list_new())){
    free(map);
    return NULL;
  }
  
  map->rules->free = simplet_rule_vfree;
  map->source = NULL;
  map->bounds = NULL;
  map->proj   = NULL;
  map->_ctx   = NULL;
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
  assert(map->valid == MAP_OK);

  if(!(map->proj = OSRNewSpatialReference(NULL)))
    return (map->valid = MAP_ERR);

  if(OSRSetFromUserInput(map->proj, proj) != OGRERR_NONE)
    return (map->valid = MAP_ERR);

  return MAP_OK;
}

int
simplet_map_set_size(simplet_map_t *map, int width, int height){
  assert(map->valid == MAP_OK);

  map->height = height;
  map->width  = width;
  return MAP_OK;
}

int
simplet_map_set_bounds(simplet_map_t *map, double maxx, double maxy, double minx, double miny){
  assert(map->valid == MAP_OK);
  if(!(map->bounds = simplet_bounds_new()))
    return (map->valid = MAP_ERR);
  simplet_bounds_extend(map->bounds, maxx, maxy);
  simplet_bounds_extend(map->bounds, minx, miny);
  return MAP_OK;
}

int
simplet_map_add_layer(simplet_map_t *map, char *datastring){
  assert(map->valid == MAP_OK);

  OGRRegisterAll();
  if(!(map->source = OGROpen(datastring, 0, NULL)))
    return (map->valid = MAP_ERR);
  return MAP_OK;
}

simplet_rule_t*
simplet_map_add_rule(simplet_map_t *map, char *sqlquery){
  assert(map->valid == MAP_OK);

  simplet_rule_t *rule;
  if(!(rule = simplet_rule_new(sqlquery))){
    map->valid = MAP_ERR;
    return NULL;
  }

  if(!simplet_list_push(map->rules, rule)){
    map->valid = MAP_ERR;
    simplet_rule_free(rule);
    return NULL;
  }

  assert(map->valid == MAP_OK);
  return rule;
}

simplet_style_t *
simplet_map_add_style(simplet_map_t *map, char *key, char *arg){
  assert(map->valid == MAP_OK);

  if(!map->rules->tail){
    map->valid = MAP_ERR;
    return NULL;
  }
  simplet_rule_t *rule = map->rules->tail->value;

  simplet_style_t *style;
  if(!(style = simplet_rule_add_style(rule, key, arg))){
    map->valid = MAP_ERR;
    return NULL;
  }

  return style;
}

int
simplet_map_isvalid(simplet_map_t *map){
  assert(map->valid == MAP_OK);

  if(map->valid == MAP_ERR)
    return MAP_ERR;

  if(!map->bounds)
    return MAP_ERR;

  if(!map->source)
    return MAP_ERR;

  if(!map->proj)
    return MAP_ERR;

  if(!map->height)
    return MAP_ERR;

  if(!map->width)
    return MAP_ERR;

  if(!map->rules->tail)
    return MAP_ERR;

  return MAP_OK;
}

int
simplet_map_render_to_png(simplet_map_t *map, char *path){
  if(simplet_map_isvalid(map) == MAP_ERR)
    return (map->valid = MAP_ERR);
    
  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, map->width, map->height);
  if(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    return (map->valid = MAP_ERR);
  cairo_t *ctx = cairo_create(surface);
  map->_ctx = ctx;
  cairo_scale(map->_ctx, map->width / map->bounds->width, map->width / map->bounds->width);
  simplet_listiter_t *iter = simplet_get_list_iter(map->rules);
  simplet_rule_t *rule;

  while((rule = simplet_list_next(iter)))
    simplet_rule_process(map, rule);

  cairo_surface_write_to_png(surface, path);
  cairo_destroy(map->_ctx);
  map->_ctx = NULL;
  return MAP_OK;
}