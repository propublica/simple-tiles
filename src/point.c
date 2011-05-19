#include <stdlib.h>
#include "point.h"

simplet_point_t*
simplet_point_new(double x, double y) {
  simplet_point_t *point;
  if(!(point = malloc(sizeof(*point))))
    return NULL;
  point->x = x;
  point->y = y;
  return point;
}

void
simplet_point_free(simplet_point_t *point){
  free(point);
}
