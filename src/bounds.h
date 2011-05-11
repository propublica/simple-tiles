#ifndef SIMPLET_BOUNDS_H
#define SIMPLET_BOUNDS_H

#include "point.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  simplet_point_t *nw;
  simplet_point_t *se;
  double width;
  double height;
} simplet_bounds_t;

simplet_bounds_t*
simplet_bounds_new();

void
simplet_bounds_extend(simplet_bounds_t *bounds, double x, double y);

void
simplet_bounds_free(simplet_bounds_t *bounds);

#ifdef __cplusplus
}
#endif

#endif