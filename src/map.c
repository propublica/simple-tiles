#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include <gdal/cpl_port.h>
#include <gdal/cpl_error.h>
#include "map.h"
#include "style.h"
#include "util.h"

static void
rule_free(void *rule){
  simplet_rule_t* tmp = rule;
  simplet_list_t* styles = tmp->styles;
  styles->free = simplet_style_vfree;
  simplet_list_free(styles);
  free(tmp->ogrsql);
  free(tmp);
}

static int
draw_polygon(simplet_map_t *map, OGRGeometryH *geom, simplet_rule_t *rule, cairo_t *ctx){
  double x;
  double y;
  double last_x;
  double last_y;
  for(int i = 0; i < OGR_G_GetGeometryCount(geom); i++){
    OGRGeometryH *subgeom = OGR_G_GetGeometryRef(geom, i);
    if(subgeom == NULL)
      continue;
    if(OGR_G_GetGeometryCount(subgeom) > 0) {
      draw_polygon(map, subgeom, rule, ctx);
      continue;
    }

    OGR_G_GetPoint(subgeom, 0, &x, &y, NULL);
    last_x = x;
    last_y = y;
    cairo_save(ctx);
    cairo_move_to(ctx, x, y);
    cairo_new_path(ctx);
    for(int j = 0; j < OGR_G_GetPointCount(subgeom) - 1; j++){
      OGR_G_GetPoint(subgeom, j, &x, &y, NULL);
      double dx;
      double dy;
      dx = fabs(last_x - x);
      dy = fabs(last_y - y);
      cairo_user_to_device_distance(ctx, &dx, &dy);
      if(dx >= 0.5 || dy >= 0.5){
        cairo_line_to(ctx, x - map->bounds->nw->x, map->bounds->nw->y - y);
        last_x = x;
        last_y = y;
      }
    }
    // ensure something is always drawn
    OGR_G_GetPoint(subgeom, OGR_G_GetPointCount(subgeom) - 1, &x, &y, NULL);
    cairo_line_to(ctx, x - map->bounds->nw->x, map->bounds->nw->y - y);
    cairo_close_path(ctx);
    simplet_apply_styles(ctx, rule->styles, 3, "weight", "fill", "stroke");
    cairo_clip(ctx);
    cairo_restore(ctx);
  }
}

// move to rules.c
static int
process_rule(simplet_map_t *map, simplet_rule_t *rule, cairo_t *ctx){
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
      draw_polygon(map, geom, rule, ctx);
      break;
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

  if(!(map->rules = simplet_list_new()))
    return NULL;
  map->rules->free = rule_free;
  map->source = NULL;
  map->bounds = NULL;
  map->proj   = NULL;
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
  if(!(rule = malloc(sizeof(*rule))))
    return (map->valid = MAP_ERR);

  if(!(rule->styles = simplet_list_new()))
    return (map->valid = MAP_ERR);

  char *sql;
  if(!(sql = simplet_copy_string(sqlquery)))
    return (map->valid = MAP_ERR);

  rule->ogrsql = sql;

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
  
  // Fail silently on unknown styles.
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
  cairo_scale(ctx, map->width / map->bounds->width, map->width / map->bounds->width);
  simplet_listiter_t *iter = simplet_get_list_iter(map->rules);
  simplet_rule_t *rule;

  while((rule = simplet_list_next(iter)))
    process_rule(map, rule, ctx);

  cairo_surface_write_to_png(surface, path);
  cairo_destroy(ctx);
  return MAP_OK;
}