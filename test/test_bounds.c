#include <simple-tiles/bounds.h>
#include "test.h"

void
is_intersecting(simplet_bounds_t *bounds, double maxx, double maxy, double minx, double miny, int flag){
  simplet_bounds_t *other = simplet_bounds_new();
  simplet_bounds_extend(other, maxx, maxy);
  simplet_bounds_extend(other, minx, miny);
  int intersects = simplet_bounds_intersects(bounds, other);
  simplet_bounds_free(other);
  assert(intersects == flag);
}

void
test_intersects(){
  simplet_bounds_t *unit = simplet_bounds_new();
  simplet_bounds_extend(unit, 1, 1);
  simplet_bounds_extend(unit, -1, -1);
  is_intersecting(unit, 2, 2, 0, 0, 1);
  is_intersecting(unit, -2, -2, 0, 0, 1);
  is_intersecting(unit, 2, -2, 0, 0, 1);
  is_intersecting(unit, -2, 2, 0, 0, 1);
  is_intersecting(unit, 2, 2, 1.1, 1.1, 0);
  simplet_bounds_free(unit);
}


TASK(bounds){
  test(intersects);
}
