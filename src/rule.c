#include <stdlib.h>
#include <assert.h>

#include "style.h"
#include "rule.h"
#include "util.h"



simplet_rule_t *
simplet_rule_new(const char *sqlquery){
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
  simplet_rule_free(rule);
}

/* arguments a little longish here */
static void
plot_path(simplet_map_t *map, OGRGeometryH geom, simplet_rule_t *rule,
          void (*cb)(simplet_map_t *map, simplet_rule_t *rule)){
  double x, y, last_x, last_y;

  for(int i = 0; i < OGR_G_GetGeometryCount(geom); i++){
    OGRGeometryH subgeom = OGR_G_GetGeometryRef(geom, i);
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
    for(int j = 0; j < OGR_G_GetPointCount(subgeom) - 1; j++){
      OGR_G_GetPoint(subgeom, j, &x, &y, NULL);
      // transform here.
      double dx = fabs(last_x - x);
      double dy = fabs(last_y - y);
      cairo_user_to_device_distance(map->_ctx, &dx, &dy);
      if(dx >= 0.25 || dy >= 0.25){
        cairo_line_to(map->_ctx, x - map->bounds->nw->x, map->bounds->nw->y - y);
        last_x = x;
        last_y = y;
      }
    }
    // ensure something is always drawn
    OGR_G_GetPoint(subgeom, OGR_G_GetPointCount(subgeom) - 1, &x, &y, NULL);
    cairo_line_to(map->_ctx, x - map->bounds->nw->x, map->bounds->nw->y - y);
  }
  (*cb)(map, rule);
}

static void
prepare_path(simplet_map_t *map, OGRGeometryH geom, simplet_rule_t *rule,
            void (*cb)(simplet_map_t *map, simplet_rule_t *rule)){
  cairo_save(map->_ctx);
  cairo_new_path(map->_ctx);
  plot_path(map, geom, rule, cb);
  cairo_close_path(map->_ctx);
  cairo_clip(map->_ctx);
  cairo_restore(map->_ctx);
}

static void
plot_point(simplet_map_t *map, OGRGeometryH geom, simplet_rule_t *rule,
          void (*cb)(simplet_map_t *map, simplet_rule_t *rule)){
  double x, y;

  simplet_style_t *style;
  style = simplet_lookup_style(rule->styles, "radius");
  if(style == NULL)
    return;

  double r = strtod(style->arg, NULL), dy = 0;

  cairo_device_to_user_distance(map->_ctx, &r, &dy);
  cairo_save(map->_ctx);
  for(int i = 0; i < OGR_G_GetPointCount(geom); i++){
    OGR_G_GetPoint(geom, i, &x, &y, NULL);
    cairo_arc(map->_ctx, x - map->bounds->nw->x, map->bounds->nw->y - y, r, 0., 2 * M_PI);
  }
  (*cb)(map, rule);

  cairo_close_path(map->_ctx);
  cairo_clip(map->_ctx);
  cairo_restore(map->_ctx);
}

static void
finish_polygon(simplet_map_t *map, simplet_rule_t *rule){
  simplet_apply_styles(map->_ctx, rule->styles,
                       "line-join", "line-cap", "weight", "fill", "stroke", NULL);
}

static void
finish_linestring(simplet_map_t *map, simplet_rule_t *rule){
  simplet_apply_styles(map->_ctx, rule->styles,
                       "line-join", "line-cap", "weight", "fill", NULL);
}

static void
finish_point(simplet_map_t *map, simplet_rule_t *rule){
  simplet_apply_styles(map->_ctx, rule->styles,
                       "line-join", "line-cap", "weight", "fill", "stroke", NULL);
}


static void
dispatch(simplet_map_t *map, OGRGeometryH geom, simplet_rule_t *rule){
  switch(OGR_G_GetGeometryType(geom)){
    case wkbPolygon:
    case wkbMultiPolygon:
      prepare_path(map, geom, rule, finish_polygon);
      break;
    case wkbLineString:
    case wkbMultiLineString:
      prepare_path(map, geom, rule, finish_linestring);
      break;
    case wkbPoint:
    case wkbMultiPoint:
      plot_point(map, geom, rule, finish_point);
      break;
    case wkbGeometryCollection:
      for(int i = 0; i < OGR_G_GetGeometryCount(geom); i++){
        OGRGeometryH subgeom = OGR_G_GetGeometryRef(geom, i);
        if(subgeom == NULL)
          continue;
        dispatch(map, subgeom, rule);
      }
      break;
    default:
      ;
  }
}


int
simplet_rule_process(simplet_rule_t *rule, simplet_layer_t *layer, simplet_map_t *map){
  OGRGeometryH bounds = simplet_bounds_to_ogr(map->bounds, map->proj);
  OGRLayerH olayer;
  if(!(olayer = OGR_DS_GetLayer(layer->source, 0)))
    return 0;

  OGRSpatialReferenceH srs;
  if(!(srs = OGR_L_GetSpatialRef(olayer)))
    return 0;

  OGR_G_TransformTo(bounds, srs);
  olayer = OGR_DS_ExecuteSQL(layer->source, rule->ogrsql, bounds, "");
  if(!olayer)
    return 0;

  bounds = simplet_bounds_to_ogr(map->bounds, map->proj);
  simplet_bounds_t* tmpb = map->bounds;
  if(!(map->bounds = simplet_bounds_from_ogr(bounds)))
    return 0;

  simplet_style_t *seemless = simplet_lookup_style(rule->styles, "seemless");
  cairo_t *tmp;
  cairo_surface_t *surface;
  if(seemless){
    tmp = map->_ctx;
    surface = cairo_surface_create_similar(cairo_get_target(map->_ctx),
                    CAIRO_CONTENT_COLOR_ALPHA, map->width, map->height);
    if(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
      return 0;
    map->_ctx = cairo_create(surface);
    cairo_set_operator(map->_ctx, CAIRO_OPERATOR_SATURATE);
  }

  cairo_scale(map->_ctx, map->width / map->bounds->width, map->width / map->bounds->width);

  OGRFeatureH feature;
  while((feature = OGR_L_GetNextFeature(olayer))){
    OGRGeometryH geom = OGR_F_GetGeometryRef(feature);
    if(geom == NULL)
      continue;
    OGR_G_TransformTo(geom, map->proj);
    dispatch(map, geom, rule);
    OGR_F_Destroy(feature);
  }

  cairo_scale(map->_ctx, map->bounds->width / map->width, map->bounds->width / map->width);

  if(seemless){
    cairo_set_source_surface(tmp, surface, 0, 0);
    cairo_paint(tmp);
    cairo_destroy(map->_ctx);
    map->_ctx = tmp;
    cairo_surface_destroy(surface);
  }

  simplet_bounds_free(map->bounds);
  map->bounds = tmpb;
  OGR_DS_ReleaseResultSet(layer->source, olayer);
  return 1;
}

simplet_style_t*
simplet_rule_add_style(simplet_rule_t *rule, const char *key, const char *arg){
  simplet_style_t *style;
  if(!(style = simplet_style_new(key, arg)))
    return NULL;

  if(!simplet_list_push(rule->styles, style)){
    simplet_style_free(style);
    return NULL;
  }

  return style;
}

