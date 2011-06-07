#include <stdlib.h>

#include "style.h"
#include "filter.h"
#include "util.h"

simplet_filter_t *
simplet_filter_new(const char *sqlquery){
  simplet_filter_t *filter;
  if(!(filter = malloc(sizeof(*filter))))
    return NULL;

  if(!(filter->styles = simplet_list_new())){
    free(filter);
    return NULL;
  }

  filter->_bounds = NULL;
  filter->_ctx    = NULL;
  filter->ogrsql  = simplet_copy_string(sqlquery);
  return filter;
}

void
simplet_filter_free(simplet_filter_t *filter){
  simplet_list_t* styles = filter->styles;
  styles->free = simplet_style_vfree;
  simplet_list_free(styles);

  if(filter->_bounds)
    simplet_bounds_free(filter->_bounds);

  if(filter->_ctx)
    cairo_destroy(filter->_ctx);

  free(filter->ogrsql);
  free(filter);
}

void
simplet_filter_vfree(void *filter){
  simplet_filter_free(filter);
}

static void
plot_path(OGRGeometryH geom, simplet_filter_t *filter,
          void (*cb)(simplet_filter_t *filter)){
  double x, y, last_x, last_y;
  for(int i = 0; i < OGR_G_GetGeometryCount(geom); i++){
    OGRGeometryH subgeom = OGR_G_GetGeometryRef(geom, i);
    if(subgeom == NULL)
      continue;

    if(OGR_G_GetGeometryCount(subgeom) > 0) {
      plot_path(subgeom, filter, cb);
      continue;
    }

    OGR_G_GetPoint(subgeom, 0, &x, &y, NULL);
    last_x = x;
    last_y = y;
    cairo_move_to(filter->_ctx, x - filter->_bounds->nw->x,  filter->_bounds->nw->y - y);

    for(int j = 0; j < OGR_G_GetPointCount(subgeom) - 1; j++){
      OGR_G_GetPoint(subgeom, j, &x, &y, NULL);
      double dx = fabs(last_x - x);
      double dy = fabs(last_y - y);
      cairo_user_to_device_distance(filter->_ctx, &dx, &dy);
      if(dx >= 0.25 || dy >= 0.25){
        cairo_line_to(filter->_ctx, x - filter->_bounds->nw->x, filter->_bounds->nw->y - y);
        last_x = x;
        last_y = y;
      }
    }
    // ensure something is always drawn
    OGR_G_GetPoint(subgeom, OGR_G_GetPointCount(subgeom) - 1, &x, &y, NULL);
    cairo_line_to(filter->_ctx, x - filter->_bounds->nw->x, filter->_bounds->nw->y - y);
  }
  (*cb)(filter);
}

static void
prepare_path(OGRGeometryH geom, simplet_filter_t *filter,
            void (*cb)(simplet_filter_t *filter)){
  cairo_save(filter->_ctx);
  cairo_new_path(filter->_ctx);
  plot_path(geom, filter, cb);
  cairo_close_path(filter->_ctx);
  cairo_clip(filter->_ctx);
  cairo_restore(filter->_ctx);
}

static void
plot_point(OGRGeometryH geom, simplet_filter_t *filter,
          void (*cb)(simplet_filter_t *filter)){
  double x, y;

  simplet_style_t *style = simplet_lookup_style(filter->styles, "radius");
  if(style == NULL)
    return;

  double r = strtod(style->arg, NULL), dy = 0;

  cairo_device_to_user_distance(filter->_ctx, &r, &dy);
  cairo_save(filter->_ctx);
  for(int i = 0; i < OGR_G_GetPointCount(geom); i++){
    OGR_G_GetPoint(geom, i, &x, &y, NULL);
    cairo_arc(filter->_ctx, x - filter->_bounds->nw->x, filter->_bounds->nw->y - y, r, 0., 2 * M_PI);
  }
  (*cb)(filter);

  cairo_close_path(filter->_ctx);
  cairo_clip(filter->_ctx);
  cairo_restore(filter->_ctx);
}

static void
finish_polygon(simplet_filter_t *filter){
  simplet_apply_styles(filter->_ctx, filter->styles,
                       "line-join", "line-cap", "weight", "fill", "stroke", NULL);
}

static void
finish_linestring(simplet_filter_t *filter){
  simplet_apply_styles(filter->_ctx, filter->styles,
                       "line-join", "line-cap", "weight", "fill", NULL);
}

static void
finish_point(simplet_filter_t *filter){
  simplet_apply_styles(filter->_ctx, filter->styles,
                       "line-join", "line-cap", "weight", "fill", "stroke", NULL);
}


static void
dispatch(OGRGeometryH geom, simplet_filter_t *filter){
  switch(OGR_G_GetGeometryType(geom)){
    case wkbPolygon:
    case wkbMultiPolygon:
      prepare_path(geom, filter, finish_polygon);
      break;
    case wkbLineString:
    case wkbMultiLineString:
      prepare_path(geom, filter, finish_linestring);
      break;
    case wkbPoint:
    case wkbMultiPoint:
      plot_point(geom, filter, finish_point);
      break;
    case wkbGeometryCollection:
      for(int i = 0; i < OGR_G_GetGeometryCount(geom); i++){
        OGRGeometryH subgeom = OGR_G_GetGeometryRef(geom, i);
        if(subgeom == NULL)
          continue;
        dispatch(subgeom, filter);
      }
      break;
    default:
      ;
  }
}

/* this function is way too hairy */
int
simplet_filter_process(simplet_filter_t *filter, simplet_layer_t *layer, simplet_map_t *map){
  OGRLayerH olayer;
  if(!(olayer = OGR_DS_GetLayer(layer->_source, 0)))
    return 0;

  OGRSpatialReferenceH srs;
  if(!(srs = OGR_L_GetSpatialRef(olayer)))
    return 0;

  OGRGeometryH bounds = simplet_bounds_to_ogr(map->bounds, map->proj);
  OGR_G_TransformTo(bounds, srs);
  olayer = OGR_DS_ExecuteSQL(layer->_source, filter->ogrsql, bounds, "");
  if(!olayer)
    return 0;

  OGRCoordinateTransformationH transform;
  if(!(transform = OCTNewCoordinateTransformation(srs, map->proj)))
    return 0;

  cairo_surface_t *surface = cairo_surface_create_similar(cairo_get_target(map->_ctx),
                                  CAIRO_CONTENT_COLOR_ALPHA, map->width, map->height);
  if(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    return 0;
  filter->_ctx = cairo_create(surface);

  simplet_style_t *seamless = simplet_lookup_style(filter->styles, "seamless");
  if(seamless)
    cairo_set_operator(filter->_ctx, CAIRO_OPERATOR_SATURATE);

  filter->_bounds = map->bounds;
  cairo_scale(filter->_ctx, map->width / filter->_bounds->width, map->width / filter->_bounds->width);

  OGRFeatureH feature;
  while((feature = OGR_L_GetNextFeature(olayer))){
    OGRGeometryH geom = OGR_F_GetGeometryRef(feature);
    if(geom == NULL)
      continue;
    OGR_G_Transform(geom, transform);
    dispatch(geom, filter);
    OGR_F_Destroy(feature);
  }

  cairo_scale(filter->_ctx, filter->_bounds->width / map->width, filter->_bounds->width / map->width);
  filter->_bounds = NULL;

  cairo_set_source_surface(map->_ctx, surface, 0, 0);
  cairo_paint(map->_ctx);
  cairo_destroy(filter->_ctx);
  filter->_ctx = NULL;
  cairo_surface_destroy(surface);
  OGR_DS_ReleaseResultSet(layer->_source, olayer);
  return 1;
}

simplet_style_t*
simplet_filter_add_style(simplet_filter_t *filter, const char *key, const char *arg){
  simplet_style_t *style;
  if(!(style = simplet_style_new(key, arg)))
    return NULL;

  if(!simplet_list_push(filter->styles, style)){
    simplet_style_free(style);
    return NULL;
  }

  return style;
}

