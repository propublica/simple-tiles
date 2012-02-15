#include <stdlib.h>
#include <cpl_error.h>

#include "style.h"
#include "query.h"
#include "util.h"
#include "list.h"
#include "map.h"
#include "bounds.h"
#include "text.h"
#include "error.h"

// Set up some user data functions.
SIMPLET_HAS_USER_DATA(query)

// Create and initialize a query.
simplet_query_t *
simplet_query_new(const char *sqlquery){
  simplet_query_t *query;
  if(!(query = malloc(sizeof(*query))))
    return NULL;

  memset(query, 0, sizeof(*query));

  if(!(query->styles = simplet_list_new())){
    free(query);
    return NULL;
  }

  query->error.status = SIMPLET_OK;
  query->ogrsql       = simplet_copy_string(sqlquery);
  return query;
}

// Free a void pointer pointing to a query.
void
simplet_query_vfree(void *query){
  simplet_query_free(query);
}


// Free a layer and associated queries.
void
simplet_query_free(simplet_query_t *query){
  simplet_list_t* styles = query->styles;
  simplet_list_set_item_free(styles, simplet_style_vfree);
  simplet_list_free(styles);
  free(query->ogrsql);
  free(query);
}

// Add an error function.
SIMPLET_ERROR_FUNC(query_t)

// Set the OGR SQL query on this query.
simplet_status_t
simplet_query_set_query(simplet_query_t *query, const char* query){
  free(query->ogrsql);
  if(!(query->ogrsql = simplet_copy_string(query)))
    return set_error(query, SIMPLET_OOM, "Out of memory setting query query");
  return SIMPLET_OK;
}

// Set query to a copy of the current query defined on query.
simplet_status_t
simplet_query_get_query(simplet_query_t *query, char** query){
  if(!(*query = simplet_copy_string(query->ogrsql))){
    query = NULL;
    return set_error(query, SIMPLET_OOM, "Out of memory copying query");
  }
  return SIMPLET_OK;
}

// Plot a part of a geometry on the ctx.
static void
plot_part(OGRGeometryH geom, simplet_query_t *query, cairo_t *ctx){
  // Look up whether we should be rendering a seamless path, if so we won't
  // simplify the points to protect against holes.
  simplet_style_t *seamless = simplet_lookup_style(query->styles, "seamless");
  double x, y, last_x, last_y;
  OGR_G_GetPoint(geom, 0, &x, &y, NULL);
  last_x = x;
  last_y = y;
  cairo_move_to(ctx, x, y);
  for(int j = 0; j < OGR_G_GetPointCount(geom); j++){
    OGR_G_GetPoint(geom, j, &x, &y, NULL);
    double dx = last_x - x;
    double dy = last_y - y;
    cairo_user_to_device_distance(ctx, &dx, &dy);
    // If we've moved a half a pixel and we aren't seamless we can plot the
    // line, this is a significant speed up vs no querying.
    if(seamless || (fabs(dx) >= 0.5 || fabs(dy) >= 0.5)){
      cairo_line_to(ctx, x, y);
      last_x = x;
      last_y = y;
    }
  }
  // Ensure something is always drawn, might not be necessary.
  OGR_G_GetPoint(geom, OGR_G_GetPointCount(geom) - 1, &x, &y, NULL);
  cairo_line_to(ctx, x, y);
}

// Plot a polygon.
static void
plot_polygon(OGRGeometryH geom, simplet_query_t *query, cairo_t *ctx){
  cairo_save(ctx);
  cairo_new_path(ctx);
  //  Split the polygon into sub polygons.
  for(int i = 0; i < OGR_G_GetGeometryCount(geom); i++){
    OGRGeometryH subgeom = OGR_G_GetGeometryRef(geom, i);
    if(subgeom == NULL)
      continue;

    // If the sub polygon has more child polygons recurse.
    if(OGR_G_GetGeometryCount(subgeom) > 0) {
      plot_polygon(subgeom, query, ctx);
      continue;
    }

    // Otherwise, plot the sub polygon.
    plot_part(subgeom, query, ctx);
    cairo_close_path(ctx);
  }
  cairo_close_path(ctx);

  // Apply the styles to the current path.
  simplet_apply_styles(ctx, query->styles,
                       "line-join", "line-cap", "weight", "fill", "stroke", NULL);
  cairo_clip(ctx);
  cairo_restore(ctx);
}

// Plot a point as a circle on the path.
static void
plot_point(OGRGeometryH geom, simplet_query_t *query, cairo_t *ctx){
	cairo_save(ctx);
  double x, y;

  simplet_style_t *style = simplet_lookup_style(query->styles, "radius");
  if(style == NULL)
    return;

  double r = strtod(style->arg, NULL), dy = 0;

  // Loop through the points in the geom and place them on the ctx.
  cairo_device_to_user_distance(ctx, &r, &dy);
  for(int i = 0; i < OGR_G_GetPointCount(geom); i++){
    OGR_G_GetPoint(geom, i, &x, &y, NULL);
    cairo_new_path(ctx);
    cairo_arc(ctx, x - r / 2, y - r / 2, r, 0., 2 * SIMPLET_PI);
    cairo_close_path(ctx);
  }
  // Apply some styles.
  simplet_apply_styles(ctx, query->styles,
                       "line-join", "line-cap", "weight", "fill", "stroke", NULL);
  cairo_restore(ctx);
}

// Plot a linestring.
static void
plot_line(OGRGeometryH geom, simplet_query_t *query, cairo_t *ctx){
  cairo_save(ctx);
  cairo_new_path(ctx);
  plot_part(geom, query, ctx);
  simplet_apply_styles(ctx, query->styles,
                        "line-join", "line-cap", "weight", "stroke", NULL);
  cairo_close_path(ctx);
  cairo_restore(ctx);
}

// Dispatch to the individual functions for rendering based on geometry type.
static void
dispatch(OGRGeometryH geom, simplet_query_t *query, cairo_t *ctx){
  switch(wkbFlatten(OGR_G_GetGeometryType(geom))) {
    case wkbPolygon:
      plot_polygon(geom, query, ctx);
      break;
    case wkbLinearRing:
    case wkbLineString:
      plot_line(geom, query, ctx);
      break;
    case wkbPoint:
      plot_point(geom, query, ctx);
      break;

    // For geometry collections, recurse into the individual members and
    // dispatch on them.
    case wkbMultiPoint:
    case wkbMultiPolygon:
    case wkbMultiLineString:
    case wkbGeometryCollection:
      for(int i = 0; i < OGR_G_GetGeometryCount(geom); i++){
        OGRGeometryH subgeom = OGR_G_GetGeometryRef(geom, i);
        if(subgeom == NULL)
          continue;
        dispatch(subgeom, query, ctx);
      }
      break;
    default:
      ;
  }
}

// Saturate the canvas for seamless shapes.
static void
set_seamless(simplet_list_t *styles, cairo_t *ctx){
  if(simplet_lookup_style(styles, "seamless"))
    cairo_set_operator(ctx, CAIRO_OPERATOR_SATURATE);
}

// This is the meat of rendering. In this function, we hit the actual data
// sources, perform transformation, add labels to the lithograph,
// and plot the individual geometries.
simplet_status_t
simplet_query_process(simplet_query_t *query, simplet_map_t *map,
  OGRDataSourceH source, simplet_lithograph_t *litho, cairo_t *ctx){

  // Grab a layer in order to suss out the srs
  OGRLayerH olayer;
  if(!(olayer = OGR_DS_ExecuteSQL(source, query->ogrsql, NULL, NULL))){
    int err = CPLGetLastErrorNo();
    if(!err)
      return SIMPLET_OK;
    else
      return set_error(query, SIMPLET_OGR_ERR, CPLGetLastErrorMsg());
  }

  // Try and figure out the srs.
  OGRSpatialReferenceH srs;
  if(!(srs = OGR_L_GetSpatialRef(olayer))){
    OGR_DS_ReleaseResultSet(source, olayer);
    int err = CPLGetLastErrorNo();
    if(!err)
      return SIMPLET_OK;
    else
      return set_error(query, SIMPLET_OGR_ERR, CPLGetLastErrorMsg());
  }

  // If the map has a buffer we need to grow the bounds a bit to grab more
  // data from the data source.
  OGRGeometryH bounds;
  if(simplet_map_get_buffer(map) > 0) {
    cairo_matrix_t mat;
    simplet_map_init_matrix(map, &mat);
    cairo_matrix_invert(&mat);
    double dx, dy;
    dx = dy = simplet_map_get_buffer(map);
    cairo_matrix_transform_distance(&mat, &dx, &dy);

    simplet_bounds_t *bbounds = simplet_bounds_buffer(map->bounds, dx);
    if(!bbounds) {
      OGR_DS_ReleaseResultSet(source, olayer);
      return SIMPLET_OGR_ERR;
    }
    bounds = simplet_bounds_to_ogr(bbounds, map->proj);
    free(bbounds);
  } else {
    bounds = simplet_bounds_to_ogr(map->bounds, map->proj);
  }

  // Transform the OGR bounds to the sources srs.
  OGR_G_TransformTo(bounds, srs);
  OGR_DS_ReleaseResultSet(source, olayer);

  // Execute the SQL and limit it to returning only the bounds set on the map.
  olayer = OGR_DS_ExecuteSQL(source, query->ogrsql, bounds, NULL);
  OGR_G_DestroyGeometry(bounds);
  if(!olayer)
    return set_error(query, SIMPLET_OGR_ERR, CPLGetLastErrorMsg());

  // Create a transorm to use in rendering later.
  OGRCoordinateTransformationH transform;
  if(!(transform = OCTNewCoordinateTransformation(srs, map->proj)))
    return set_error(query, SIMPLET_OGR_ERR, CPLGetLastErrorMsg());

  // Copy the original surface so we don't muss about with defaults.
  cairo_surface_t *surface = cairo_surface_create_similar(cairo_get_target(ctx),
                                  CAIRO_CONTENT_COLOR_ALPHA, map->width, map->height);
  if(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    return set_error(query, SIMPLET_CAIRO_ERR, (const char *)cairo_status_to_string(cairo_surface_status(surface)));

  // Setup seamless rendering.
  cairo_t *sub_ctx = cairo_create(surface);
  set_seamless(query->styles, sub_ctx);

  // Initialize the transformation matrix.
  cairo_matrix_t mat;
  simplet_map_init_matrix(map, &mat);
  cairo_set_matrix(sub_ctx, &mat);

  // Loop through and place the features.
  OGRFeatureH feature;
  while((feature = OGR_L_GetNextFeature(olayer))){
    OGRGeometryH geom = OGR_F_GetGeometryRef(feature);

    if(geom == NULL || OGR_G_Transform(geom, transform) != OGRERR_NONE){
      OGR_F_Destroy(feature);
      continue;
    }

    dispatch(geom, query, sub_ctx);

    // Add feature labels, this is another loop, but it should be fast enough/
    simplet_lithograph_add_placement(litho, feature, query->styles, sub_ctx);
    OGR_F_Destroy(feature);
  }

  // Cleanup.
  cairo_set_source_surface(ctx, surface, 0, 0);
  cairo_paint(ctx);
  cairo_destroy(sub_ctx);
  cairo_surface_destroy(surface);
  OGR_DS_ReleaseResultSet(source, olayer);
  OCTDestroyCoordinateTransformation(transform);
  return SIMPLET_OK;
}

// Initialize and add a new style to this query.
simplet_style_t*
simplet_query_add_style(simplet_query_t *query, const char *key, const char *arg){
  simplet_style_t *style;
  if(!(style = simplet_style_new(key, arg)))
    return NULL;

  if(!simplet_list_push(query->styles, style)){
    simplet_style_free(style);
    return NULL;
  }

  return style;
}

// Add a previously initialized style.
simplet_style_t*
simplet_query_add_style_directly(simplet_query_t *query, simplet_style_t *style){
  if(!simplet_list_push(query->styles, style)) return NULL;
  return style;
}


