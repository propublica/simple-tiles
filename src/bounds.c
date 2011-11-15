#include <stdlib.h>
#include <stdio.h>
#include "math.h"
#include "bounds.h"

void
simplet_bounds_extend(simplet_bounds_t *bounds, double x, double y){
  bounds->nw.x = fmin(x, bounds->nw.x);
  bounds->nw.y = fmax(y, bounds->nw.y);
  bounds->se.x = fmax(x, bounds->se.x);
  bounds->se.y = fmin(y, bounds->se.y);
  bounds->width  = fabs(bounds->nw.x - bounds->se.x);
  bounds->height = fabs(bounds->nw.y - bounds->se.y);
}

OGRGeometryH
simplet_bounds_to_ogr(simplet_bounds_t *bounds, OGRSpatialReferenceH proj) {
  OGRGeometryH tmpLine;
  if(!(tmpLine = OGR_G_CreateGeometry(wkbLineString)))
    return NULL;

  OGR_G_AddPoint_2D(tmpLine, bounds->nw.x, bounds->nw.y);
  OGR_G_AddPoint_2D(tmpLine, bounds->se.x, bounds->se.y);
  OGR_G_AddPoint_2D(tmpLine, bounds->nw.x, bounds->se.y);
  OGR_G_AddPoint_2D(tmpLine, bounds->se.x, bounds->nw.y);

  OGRGeometryH ogrBounds;
  if(!(ogrBounds = OGR_G_ConvexHull(tmpLine))){
    OGR_G_DestroyGeometry(tmpLine);
    return NULL;
  }

  OGR_G_AssignSpatialReference(ogrBounds, proj);
  OGR_G_DestroyGeometry(tmpLine);
  return ogrBounds;
}

int
simplet_bounds_intersects(simplet_bounds_t *bounds, simplet_bounds_t *obounds){
  return !(bounds->nw.x > obounds->se.x || bounds->nw.y < obounds->se.y
        || bounds->se.x > obounds->nw.x || bounds->se.y > obounds->nw.y);
}

simplet_bounds_t*
simplet_bounds_from_ogr(OGRGeometryH geom){
  OGRGeometryH hull;
  if(!(hull = OGR_G_ConvexHull(geom)))
    return NULL;

  simplet_bounds_t *bounds;
  if(!(bounds = simplet_bounds_new())){
    OGR_G_DestroyGeometry(hull);
    return NULL;
  }

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

void
simplet_bounds_free(simplet_bounds_t *bounds){
  free(bounds);
}

simplet_bounds_t*
simplet_bounds_new(){
  simplet_bounds_t *bounds;
  if(!(bounds = malloc(sizeof(*bounds))))
    return NULL;

  memset(bounds, 0, sizeof(*bounds));

  bounds->nw.x   = INFINITY;
  bounds->nw.y   = -INFINITY;
  bounds->se.x   = -INFINITY;
  bounds->se.y   = INFINITY;

  return bounds;
}


simplet_status_t
simplet_bounds_to_wkt(simplet_bounds_t *bounds, char **wkt){
  asprintf(wkt, "POLYGON ((%f %f, %f %f, %f %f, %f %f, %f %f))",
                  bounds->se.x, bounds->nw.y,
                  bounds->se.x, bounds->se.y,
                  bounds->nw.x, bounds->se.y,
                  bounds->nw.x, bounds->nw.y,
                  bounds->se.x, bounds->nw.y);
  return SIMPLET_OK;
}

simplet_bounds_t*
simplet_bounds_reproject(simplet_bounds_t* bounds, const char *from, const char *to){
  OGRSpatialReferenceH proj_from = OSRNewSpatialReference(NULL);
  if(OSRSetFromUserInput(proj_from, from) != OGRERR_NONE) return NULL;
  OGRGeometryH geom = simplet_bounds_to_ogr(bounds, proj_from);
  OGRSpatialReferenceH proj_to = OSRNewSpatialReference(NULL);
  if(OSRSetFromUserInput(proj_to, to) != OGRERR_NONE) return NULL;
  OGR_G_TransformTo(geom, proj_to);
  simplet_bounds_t *new_bounds = simplet_bounds_from_ogr(geom);
  OGR_G_DestroyGeometry(geom);
  OSRDestroySpatialReference(proj_from);
  OSRDestroySpatialReference(proj_to);
  return new_bounds;
}
