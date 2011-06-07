#include <stdlib.h>
#include <stdio.h>
#include "math.h"
#include "bounds.h"
#include "point.h"


void
simplet_bounds_extend(simplet_bounds_t *bounds, double x, double y){
  bounds->nw->x = fmin(x, bounds->nw->x);
  bounds->nw->y = fmax(y, bounds->nw->y);
  bounds->se->x = fmax(x, bounds->se->x);
  bounds->se->y = fmin(y, bounds->se->y);
  bounds->width  = fabs(bounds->nw->x - bounds->se->x);
  bounds->height = fabs(bounds->nw->y - bounds->se->y);
}

OGRGeometryH
simplet_bounds_to_ogr(simplet_bounds_t *bounds) {
  OGRGeometryH tmpLine;
  if(!(tmpLine = OGR_G_CreateGeometry(wkbLineString)))
    return NULL;

  OGR_G_AddPoint_2D(tmpLine, bounds->nw->x, bounds->nw->y);
  OGR_G_AddPoint_2D(tmpLine, bounds->se->x, bounds->se->y);
  OGR_G_AddPoint_2D(tmpLine, bounds->nw->x, bounds->se->y);
  OGR_G_AddPoint_2D(tmpLine, bounds->se->x, bounds->nw->y);

  OGRGeometryH ogrBounds;
  if(!(ogrBounds = OGR_G_ConvexHull(tmpLine))){
    OGR_G_DestroyGeometry(tmpLine);
    return NULL;
  }

  const char *wgs84 = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
  OGRSpatialReferenceH proj;
  if(!(proj = OSRNewSpatialReference(NULL))){
    OGR_G_DestroyGeometry(tmpLine);
    OGR_G_DestroyGeometry(ogrBounds);
    return NULL;
  }

  if(OSRSetFromUserInput(proj, wgs84) != OGRERR_NONE){
    OGR_G_DestroyGeometry(tmpLine);
    OSRDestroySpatialReference(proj);
    OGR_G_DestroyGeometry(ogrBounds);
    return NULL;
  }

  OGR_G_AssignSpatialReference(ogrBounds, proj);
  OGR_G_DestroyGeometry(tmpLine);
  return ogrBounds;
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
  simplet_point_free(bounds->nw);
  simplet_point_free(bounds->se);
  free(bounds);
}

simplet_bounds_t*
simplet_bounds_new(){
  simplet_bounds_t *bounds;
  if((bounds = malloc(sizeof(*bounds))) == NULL)
    return NULL;
  bounds->nw     = simplet_point_new(INFINITY, -INFINITY);
  bounds->se     = simplet_point_new(-INFINITY, INFINITY);
  bounds->width  = 0;
  bounds->height = 0;
  return bounds;
}
