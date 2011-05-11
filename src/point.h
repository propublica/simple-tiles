#ifndef SIMPLET_POINT_H
#define SIMPLET_POINT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  double x;
  double y;
} simplet_point_t;

simplet_point_t*
simplet_point_new(double x, double y);

void
simplet_point_free(simplet_point_t *point);

#ifdef __cplusplus
}
#endif

#endif