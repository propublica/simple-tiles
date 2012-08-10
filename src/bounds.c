#include <stdlib.h>
#include <stdio.h>
#include "math.h"
#include "bounds.h"


// Extend the bounds to include the x, y point.
void
simplet_bounds_extend(simplet_bounds_t *bounds, double x, double y){
  bounds->nw.x = fmin(x, bounds->nw.x);
  bounds->nw.y = fmax(y, bounds->nw.y);
  bounds->se.x = fmax(x, bounds->se.x);
  bounds->se.y = fmin(y, bounds->se.y);
  bounds->width  = fabs(bounds->nw.x - bounds->se.x);
  bounds->height = fabs(bounds->nw.y - bounds->se.y);
}

// Return a OGR_G_ConvexHull based on the bounds in a specified projection.
OGRGeometryH
simplet_bounds_to_ogr(simplet_bounds_t *bounds, OGRSpatialReferenceH proj) {
  OGRGeometryH tmpLine;
  if(!(tmpLine = OGR_G_CreateGeometry(wkbLineString)))
    return NULL;

  // Add all the points defining the bounds to the geometry
  OGR_G_AddPoint_2D(tmpLine, bounds->nw.x, bounds->nw.y);
  OGR_G_AddPoint_2D(tmpLine, bounds->se.x, bounds->se.y);
  OGR_G_AddPoint_2D(tmpLine, bounds->nw.x, bounds->se.y);
  OGR_G_AddPoint_2D(tmpLine, bounds->se.x, bounds->nw.y);
  OGRGeometryH tmpPoint = OGR_G_ForceToMultiPoint(tmpLine);
  // Calculate the Convex Hull
  OGRGeometryH ogrBounds;
  if(!(ogrBounds = OGR_G_ConvexHull(tmpPoint))){
    OGR_G_DestroyGeometry(tmpLine);
    return NULL;
  }

  // And assign the projection.
  OGR_G_AssignSpatialReference(ogrBounds, proj);
  OGR_G_DestroyGeometry(tmpPoint);

  return ogrBounds;
}

// Test if one bounds intersects another.
int
simplet_bounds_intersects(simplet_bounds_t *bounds, simplet_bounds_t *obounds){
  return !(bounds->nw.x > obounds->se.x || bounds->nw.y < obounds->se.y
        || bounds->se.x < obounds->nw.x || bounds->se.y > obounds->nw.y);
}

// Create a bounds from the Convex Hull of an OGR Geometry.
simplet_bounds_t*
simplet_bounds_from_ogr(OGRGeometryH geom){
  OGRGeometryH hull;

  // Grab the Convex Hull
  if(!(hull = OGR_G_ConvexHull(geom)))
    return NULL;

  // Create the bounds.
  simplet_bounds_t *bounds;
  if(!(bounds = simplet_bounds_new())){
    OGR_G_DestroyGeometry(hull);
    return NULL;
  }

  // Extend the bounds by adding the points from the Convex Hull.
  double x, y;
  for(int i = 0; i < OGR_G_GetGeometryCount(hull); i++){
    OGRGeometryH subgeom = OGR_G_GetGeometryRef(hull, i);
    if(subgeom == NULL)
      continue;
    for(int j = 0; j < OGR_G_GetPointCount(subgeom); j++){
      OGR_G_GetPoint(subgeom, j, &x, &y, NULL);
      simplet_bounds_extend(bounds, x, y);
    }
  }

  OGR_G_DestroyGeometry(hull);
  return bounds;
}

// Free the memory associated with the bounds.
void
simplet_bounds_free(simplet_bounds_t *bounds){
  if(simplet_release((simplet_retainable_t *)bounds) == 0) free(bounds);
}

// Allocate and return a new simplet_bounds_t.
simplet_bounds_t*
simplet_bounds_new(){
  simplet_bounds_t *bounds;
  if(!(bounds = malloc(sizeof(*bounds))))
    return NULL;

  memset(bounds, 0, sizeof(*bounds));

  // Set the bounds to be as big as the universe.
  bounds->nw.x = INFINITY;
  bounds->nw.y = -INFINITY;
  bounds->se.x = -INFINITY;
  bounds->se.y = INFINITY;

  simplet_retain((simplet_retainable_t *)bounds);
  return bounds;
}

// Convert a bounds to a Well Known Text string and store it in **wkt
simplet_status_t
simplet_bounds_to_wkt(simplet_bounds_t *bounds, char **wkt){
  int ret = asprintf(wkt, "POLYGON ((%f %f, %f %f, %f %f, %f %f, %f %f))",
                  bounds->se.x, bounds->nw.y,
                  bounds->se.x, bounds->se.y,
                  bounds->nw.x, bounds->se.y,
                  bounds->nw.x, bounds->nw.y,
                  bounds->se.x, bounds->nw.y);
  if(ret > -1) return SIMPLET_OK;
  return SIMPLET_ERR;
}

simplet_bounds_t*
simplet_bounds_buffer(simplet_bounds_t* bounds, double extend){
  simplet_bounds_t* new;
  if(!(new = simplet_bounds_new()))
    return NULL;

  simplet_bounds_extend(new, bounds->nw.x - extend, bounds->nw.y + extend);
  simplet_bounds_extend(new, bounds->se.x + extend, bounds->se.y - extend);

  return new;
}

// Reproject a simplet_bounds_t into a new projection and return a new copy.
simplet_bounds_t*
simplet_bounds_reproject(simplet_bounds_t* bounds, const char *from, const char *to){
  // Create a new spatial reference for `from`.
  OGRSpatialReferenceH proj_from = OSRNewSpatialReference(NULL);
  if(OSRSetFromUserInput(proj_from, from) != OGRERR_NONE) return NULL;

  // Translate the bounds to an OGR object.
  OGRGeometryH geom = simplet_bounds_to_ogr(bounds, proj_from);
  OGRSpatialReferenceH proj_to = OSRNewSpatialReference(NULL);

  // Create a new spatial reference for `to`
  if(OSRSetFromUserInput(proj_to, to) != OGRERR_NONE) return NULL;
  OGR_G_TransformTo(geom, proj_to);

  // Create a bounds object from the OGR object.
  simplet_bounds_t *new_bounds = simplet_bounds_from_ogr(geom);
  OGR_G_DestroyGeometry(geom);
  OSRDestroySpatialReference(proj_from);
  OSRDestroySpatialReference(proj_to);
  return new_bounds;
}
