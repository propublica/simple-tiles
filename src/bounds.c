#include <stdlib.h>
#include <stdio.h>
#include "math.h"
#include "bounds.h"


void
simplet_bounds_extend(simplet_bounds_t *bounds, double x, double y){
  bounds->nw->x = fmin(x, bounds->nw->x);
  bounds->nw->y = fmax(y, bounds->nw->y);
  bounds->se->x = fmax(x, bounds->se->x);
  bounds->se->y = fmin(y, bounds->se->y);
  bounds->width  = fabs(bounds->nw->x - bounds->se->x);
  bounds->height = fabs(bounds->nw->y - bounds->se->y);
}

OGRGeometryH *
simplet_bounds_to_ogr(simplet_bounds_t *bounds, OGRSpatialReferenceH *proj) {
  char *fmt = "POLYGON ((%d %d, %d %d, %d %d, %d %d))";
  char *str;
  int len = snprintf(NULL, 0, fmt, bounds->nw->x,
                                   bounds->nw->y,
                                   bounds->se->x,
                                   bounds->nw->y,
                                   bounds->se->x,
                                   bounds->se->y,
                                   bounds->nw->x,
                                   bounds->se->y);
  if(!(str = malloc((len + 1) * sizeof(char))))
    return NULL;
  snprintf(str, len + 1, fmt, bounds->nw->x,
                              bounds->nw->y,
                              bounds->se->x,
                              bounds->nw->y,
                              bounds->se->x,
                              bounds->se->y,
                              bounds->nw->x,
                              bounds->se->y);
  
  OGRGeometryH *ogrbounds;
  if((OGR_G_CreateFromWkt(&str, proj, ogrbounds) != OGRERR_NONE))
    return NULL;
  free(str);
  
  return ogrbounds;
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
