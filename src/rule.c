#include <stdlib.h>
#include <assert.h>

#include "style.h"
#include "rule.h"
#include "util.h"


simplet_rule_t *
simplet_rule_new(char *sqlquery){
  simplet_rule_t *rule;
  if(!(rule = malloc(sizeof(*rule))))
    return NULL;

  if(!(rule->styles = simplet_list_new())){
    free(rule);
    return NULL;
  }
  
  rule->ogrsql = simplet_copy_string(sqlquery);
  return rule;
}

void
simplet_rule_free(simplet_rule_t *rule){
  simplet_list_t* styles = rule->styles;
  styles->free = simplet_style_vfree;
  simplet_list_free(styles);
  free(rule->ogrsql);
  free(rule);
}

void
simplet_rule_vfree(void *rule){
  simplet_rule_t *tmp = rule;
  simplet_rule_free(tmp);
}

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
    cairo_move_to(map->_ctx, x - map->bounds->nw->x,  map->bounds->nw->y - y);
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
  
  simplet_style_t *style;
  style = simplet_lookup_style(rule->styles, "radius");
  if(style == NULL)
    return;

  cairo_save(map->_ctx);
  for(int i = 0; i < OGR_G_GetPointCount(geom); i++){
    OGR_G_GetPoint(geom, i, &x, &y, NULL);
    double r = strtod(style->arg, NULL);
    double dy = 0;
    cairo_device_to_user_distance(map->_ctx, &r, &dy);
    cairo_arc(map->_ctx, x - map->bounds->nw->x, map->bounds->nw->y - y, r, 0., 2 * M_PI);
  }
  (*cb)(map, rule);
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
  simplet_apply_styles(map->_ctx, rule->styles, 3, "weight", "fill", "stroke");
}


static void
dispatch(simplet_map_t *map, OGRGeometryH *geom, simplet_rule_t *rule){
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
    case wkbGeometryCollection:
      for(int i = 0; i < OGR_G_GetGeometryCount(geom); i++){
        OGRGeometryH *subgeom = OGR_G_GetGeometryRef(geom, i);
        if(subgeom == NULL)
          continue;
        dispatch(map, subgeom, rule);
      }
      break;
    default:
      ;
  }
}

// move to rules.c
int
simplet_rule_process(simplet_map_t *map, simplet_rule_t *rule){
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
    dispatch(map, geom, rule);
    OGR_F_Destroy(feature);
  }
  OGR_DS_ReleaseResultSet(map->source, layer);
  return 1;
}

