#include <stdlib.h>
#include <cpl_error.h>

#include "style.h"
#include "filter.h"
#include "util.h"
#include "error.h"

simplet_filter_t *
simplet_filter_new(const char *sqlquery){
  simplet_filter_t *filter;
  if(!(filter = malloc(sizeof(*filter))))
    return NULL;

  if(!(filter->styles = simplet_list_new())){
    free(filter);
    return NULL;
  }

  filter->error.status = SIMPLET_OK;
  filter->ogrsql       = simplet_copy_string(sqlquery);
  return filter;
}

void
simplet_filter_free(simplet_filter_t *filter){
  simplet_list_t* styles = filter->styles;
  simplet_list_set_item_free(styles, simplet_style_vfree);
  simplet_list_free(styles);
  free(filter->ogrsql);
  free(filter);
}

SIMPLET_ERROR_FUNC(filter_t)

simplet_status_t
simplet_filter_set_query(simplet_filter_t *filter, const char* query){
  free(filter->ogrsql);
  if(!(filter->ogrsql = simplet_copy_string(query)))
    return set_error(filter, SIMPLET_OOM, "Out of memory setting filter query");
  return SIMPLET_OK;
}

simplet_status_t
simplet_filter_get_query(simplet_filter_t *filter, char** query){
  if(!(*query = simplet_copy_string(filter->ogrsql))){
    query = NULL;
    return set_error(filter, SIMPLET_OOM, "Out of memory copying query");
  }
  return SIMPLET_OK;
}

void
simplet_filter_vfree(void *filter){
  simplet_filter_free(filter);
}

static void
plot_part(OGRGeometryH geom, simplet_filter_t *filter, cairo_t *ctx, simplet_bounds_t *bounds){
  simplet_style_t *seamless = simplet_lookup_style(filter->styles, "seamless");
  double x, y, last_x, last_y;
  OGR_G_GetPoint(geom, 0, &x, &y, NULL);
  last_x = x;
  last_y = y;
  cairo_move_to(ctx, x - bounds->nw.x,
                bounds->nw.y - y);
  for(int j = 0; j < OGR_G_GetPointCount(geom); j++){
    OGR_G_GetPoint(geom, j, &x, &y, NULL);
    double dx = fabs(last_x - x);
    double dy = fabs(last_y - y);
    cairo_user_to_device_distance(ctx, &dx, &dy);

    if(seamless || (dx >= 0.25 || dy >= 0.25)){
      cairo_line_to(ctx, x - bounds->nw.x,
                    bounds->nw.y - y);
      last_x = x;
      last_y = y;
    }
  }
  // ensure something is always drawn
  OGR_G_GetPoint(geom, OGR_G_GetPointCount(geom) - 1, &x, &y, NULL);
  cairo_line_to(ctx, x - bounds->nw.x,
                              bounds->nw.y - y);
}

static void
plot_polygon(OGRGeometryH geom, simplet_filter_t *filter, cairo_t *ctx, simplet_bounds_t *bounds){
  cairo_save(ctx);
  cairo_new_path(ctx);
  for(int i = 0; i < OGR_G_GetGeometryCount(geom); i++){
    OGRGeometryH subgeom = OGR_G_GetGeometryRef(geom, i);
    if(subgeom == NULL)
      continue;

    if(OGR_G_GetGeometryCount(subgeom) > 0) {
      plot_polygon(subgeom, filter, ctx, bounds);
      continue;
    }

    plot_part(subgeom, filter, ctx, bounds);
    cairo_close_path(ctx);
  }
  cairo_close_path(ctx);
  simplet_apply_styles(ctx, filter->styles,
                       "line-join", "line-cap", "weight", "fill", "stroke", NULL);
  cairo_clip(ctx);
  cairo_restore(ctx);
}

static void
plot_point(OGRGeometryH geom, simplet_filter_t *filter, cairo_t *ctx, simplet_bounds_t *bounds){
	cairo_save(ctx);
  double x, y;

  simplet_style_t *style = simplet_lookup_style(filter->styles, "radius");
  if(style == NULL)
    return;

  double r = strtod(style->arg, NULL), dy = 0;

  cairo_device_to_user_distance(ctx, &r, &dy);
  cairo_save(ctx);
  for(int i = 0; i < OGR_G_GetPointCount(geom); i++){
    OGR_G_GetPoint(geom, i, &x, &y, NULL);
    cairo_new_path(ctx);
    cairo_arc(ctx, x - bounds->nw.x - r / 2,
              bounds->nw.y - y - r / 2, r, 0., 2 * M_PI);
    cairo_close_path(ctx);
  }
  simplet_apply_styles(ctx, filter->styles,
                       "line-join", "line-cap", "weight", "fill", "stroke", NULL);
  cairo_restore(ctx);
}

static void
plot_line(OGRGeometryH geom, simplet_filter_t *filter, cairo_t *ctx, simplet_bounds_t *bounds){
  cairo_save(ctx);
  cairo_new_path(ctx);
  plot_part(geom, filter, ctx, bounds);
  simplet_apply_styles(ctx, filter->styles,
                        "line-join", "line-cap", "weight", "stroke", NULL);
  cairo_close_path(ctx);
  cairo_restore(ctx);
}

static void
dispatch(OGRGeometryH geom, simplet_filter_t *filter, cairo_t *ctx, simplet_bounds_t *bounds){
  switch(wkbFlatten(OGR_G_GetGeometryType(geom))){
    case wkbPolygon:
      plot_polygon(geom, filter, ctx, bounds);
      break;
    case wkbLinearRing:
    case wkbLineString:
      plot_line(geom, filter, ctx, bounds);
      break;
    case wkbPoint:
      plot_point(geom, filter, ctx, bounds);
      break;
    case wkbMultiPoint:
    case wkbMultiPolygon:
    case wkbMultiLineString:
    case wkbGeometryCollection:
      for(int i = 0; i < OGR_G_GetGeometryCount(geom); i++){
        OGRGeometryH subgeom = OGR_G_GetGeometryRef(geom, i);
        if(subgeom == NULL)
          continue;
        dispatch(subgeom, filter, ctx, bounds);
      }
      break;
    default:
      ;
  }
}


static void
set_seamless(simplet_list_t *styles, cairo_t *ctx){
  simplet_style_t *seamless = simplet_lookup_style(styles, "seamless");

  if(seamless)
    cairo_set_operator(ctx, CAIRO_OPERATOR_SATURATE);
}


simplet_status_t
simplet_filter_process(simplet_filter_t *filter, simplet_map_t *map, OGRDataSourceH source, cairo_t *ctx){

  OGRLayerH olayer;
  if(!(olayer = OGR_DS_ExecuteSQL(source, filter->ogrsql, NULL, NULL))){
    int err = CPLGetLastErrorNo();
    if(!err)
      return SIMPLET_OK;
    else
      return set_error(filter, SIMPLET_OGR_ERR, CPLGetLastErrorMsg());
  }

  OGRSpatialReferenceH srs;
  if(!(srs = OGR_L_GetSpatialRef(olayer))){
    OGR_DS_ReleaseResultSet(source, olayer);
    int err = CPLGetLastErrorNo();
    if(!err)
      return SIMPLET_OK;
    else
      return set_error(filter, SIMPLET_OGR_ERR, CPLGetLastErrorMsg());
  }

  OGRGeometryH bounds = simplet_bounds_to_ogr(map->bounds, map->proj);
  OGR_G_TransformTo(bounds, srs);
  OGR_DS_ReleaseResultSet(source, olayer);

  olayer = OGR_DS_ExecuteSQL(source, filter->ogrsql, bounds, NULL);
  OGR_G_DestroyGeometry(bounds);
  if(!olayer)
    return set_error(filter, SIMPLET_OGR_ERR, CPLGetLastErrorMsg());

  OGRCoordinateTransformationH transform;
  if(!(transform = OCTNewCoordinateTransformation(srs, map->proj)))
    return set_error(filter, SIMPLET_OGR_ERR, CPLGetLastErrorMsg());

  cairo_surface_t *surface = cairo_surface_create_similar(cairo_get_target(ctx),
                                  CAIRO_CONTENT_COLOR_ALPHA, map->width, map->height);
  if(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    return set_error(filter, SIMPLET_CAIRO_ERR, (const char *)cairo_status_to_string(cairo_surface_status(surface)));

  cairo_t *sub_ctx = cairo_create(surface);
  set_seamless(filter->styles, sub_ctx);

  simplet_bounds_t *mbounds = map->bounds;
  cairo_scale(sub_ctx, map->width / mbounds->width, map->width / mbounds->width);

  OGRFeatureH feature;
  while((feature = OGR_L_GetNextFeature(olayer))){
    OGRGeometryH geom = OGR_F_GetGeometryRef(feature);
    if(geom == NULL) continue;
    if(OGR_G_Transform(geom, transform) != OGRERR_NONE) continue;
    dispatch(geom, filter, sub_ctx, mbounds);
    OGR_F_Destroy(feature);
  }
  cairo_scale(ctx, mbounds->width / map->width, mbounds->width / map->width);
  cairo_set_source_surface(ctx, surface, 0, 0);
  cairo_paint(ctx);
  cairo_destroy(sub_ctx);
  cairo_surface_destroy(surface);
  OGR_DS_ReleaseResultSet(source, olayer);
  OCTDestroyCoordinateTransformation(transform);
  return SIMPLET_OK;
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
