#ifndef __SIMPLET_POINT_H__
#define __SIMPLET_POINT_H__

#ifdef __cplusplus
extern "C" {
#endif
typedef struct simplet_point {
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