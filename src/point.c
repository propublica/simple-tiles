#include <stdlib.h>
#include "point.h"

simplet_point_t
simplet_point_new(double x, double y) {
  simplet_point_t point;
  point.x = x;
  point.y = y;
  return point;
}

