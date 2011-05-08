#include <stdlib.h>
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
