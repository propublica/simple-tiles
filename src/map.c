#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "map.h"
#include "style.h"
#include "rule.h"
#include "util.h"



/* arguments a little longish here */
static void
plot_path(simplet_map_t *map, OGRGeometryH *geom, simplet_rule_t *rule, 
          void (*cb)(simplet_map_t *map, simplet_rule_t *rule)){
  double x;
  double y;
  double last_x;
  double last_y;
  cairo_save(map->_ctx);
  for(int i = 0; i < OGR_G_GetGeometryCount(geom); i++){
    OGRGeometryH *subgeom = OGR_G_GetGeometryRef(geom, i);
    if(subgeom == NULL)
      continue;
    if(OGR_G_GetGeometryCount(subgeom) > 0) {
      plot_path(map, subgeom, rule, cb);
      continue;
    }

    OGR_G_GetPoint(subgeom, 0, &x, &y, NULL);
    last_x = x;
    last_y = y;
    cairo_move_to(map->_ctx, x, y);
    cairo_new_path(map->_ctx);
    for(int j = 0; j < OGR_G_GetPointCount(subgeom) - 1; j++){
      OGR_G_GetPoint(subgeom, j, &x, &y, NULL);
      double dx;
      double dy;
      dx = fabs(last_x - x);
      dy = fabs(last_y - y);
      cairo_user_to_device_distance(map->_ctx, &dx, &dy);
      if(dx >= 0.5 || dy >= 0.5){
        cairo_line_to(map->_ctx, x - map->bounds->nw->x, map->bounds->nw->y - y);
        last_x = x;
        last_y = y;
      }
    }
    // ensure something is always drawn
    OGR_G_GetPoint(subgeom, OGR_G_GetPointCount(subgeom) - 1, &x, &y, NULL);
    cairo_line_to(map->_ctx, x - map->bounds->nw->x, map->bounds->nw->y - y);
    (*cb)(map, rule);
  }
  cairo_clip(map->_ctx);
  cairo_restore(map->_ctx);
}

static void
plot_point(simplet_map_t *map, OGRGeometryH *geom, simplet_rule_t *rule, 
          void (*cb)(simplet_map_t *map, simplet_rule_t *rule)){
  double x;
  double y;
  cairo_save(map->_ctx);
  for(int i = 0; i < OGR_G_GetGeometryCount(geom); i++){
    OGRGeometryH *subgeom = OGR_G_GetGeometryRef(geom, i);
    if(subgeom == NULL)
      continue;
    if(OGR_G_GetGeometryCount(subgeom) > 0) {
      plot_point(map, subgeom, rule, cb);
      continue;
    }
     // should only run once, but just to be sure
    for(int j = 0; j < OGR_G_GetPointCount(subgeom); j++){
      cairo_move_to(map->_ctx, x, y);
      simplet_style_t *style;
      style = simplet_lookup_style(rule->styles, "radius");
      if(style == NULL)
        continue;      
      cairo_arc(map->_ctx, x - map->bounds->nw->x, map->bounds->nw->y - y, strtod(style->arg, NULL), 0., 2 * M_PI);
    }
    (*cb)(map, rule);
  }
  cairo_clip(map->_ctx);
  cairo_restore(map->_ctx);
}

static void
finish_polygon(simplet_map_t *map, simplet_rule_t *rule){
  cairo_close_path(map->_ctx);
  simplet_apply_styles(map->_ctx, rule->styles, 3, "weight", "fill", "stroke");
}

static void
finish_linestring(simplet_map_t *map, simplet_rule_t *rule){
  simplet_apply_styles(map->_ctx, rule->styles, 2, "weight", "fill");
}

static void
finish_point(simplet_map_t *map, simplet_rule_t *rule){
  cairo_close_path(map->_ctx);
  simplet_apply_styles(map->_ctx, rule->styles, 4, "weight", "fill", "stroke");
}

// move to rules.c
static int
process_rule(simplet_map_t *map, simplet_rule_t *rule){
  OGRGeometryH *bounds = simplet_bounds_to_ogr(map->bounds, map->proj);
  assert(bounds != NULL);
  OGRLayerH *layer = OGR_DS_ExecuteSQL(map->source, rule->ogrsql, bounds, "");
  OGR_G_DestroyGeometry(bounds);
  if(!layer)
    return 0;

  OGRFeatureH *feature;
  while((feature = OGR_L_GetNextFeature(layer))){
    OGRGeometryH *geom = OGR_F_GetGeometryRef(feature);
    if(geom == NULL)
      continue;
    switch(OGR_G_GetGeometryType(geom)){
      case wkbPolygon:
      case wkbMultiPolygon:
        plot_path(map, geom, rule, finish_polygon);
        break;
      case wkbLineString:
      case wkbMultiLineString:
        plot_path(map, geom, rule, finish_linestring);
        break;
      case wkbPoint:
      case wkbMultiPoint:
        plot_point(map, geom, rule, finish_point);
        break;
      default:
        ;
    }
    OGR_F_Destroy(feature);
  }
  OGR_DS_ReleaseResultSet(map->source, layer);
  return 1;
}


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

int
simplet_map_add_rule(simplet_map_t *map, char *sqlquery){
  assert(map->valid == MAP_OK);

  // move to rules.c
  simplet_rule_t *rule;
  if(!(rule = simplet_rule_new(sqlquery)))
    return (map->valid = MAP_ERR);

  if(!simplet_list_push(map->rules, rule))
    return (map->valid = MAP_ERR);

  assert(map->valid == MAP_OK);
  return MAP_OK;
}

int
simplet_map_add_style(simplet_map_t *map, char *key, char *arg){
  assert(map->valid == MAP_OK);

  if(!map->rules->tail)
    return (map->valid = MAP_ERR);
  simplet_rule_t *rule = map->rules->tail->value;
  
  simplet_style_t *style;
  if(!(style = simplet_style_new(key, arg)))
    return (map->valid = MAP_ERR);

  simplet_list_push(rule->styles, style);

  return MAP_OK;
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
    process_rule(map, rule);

  cairo_surface_write_to_png(surface, path);
  cairo_destroy(map->_ctx);
  map->_ctx = NULL;
  return MAP_OK;
}