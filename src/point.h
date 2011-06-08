#ifndef _SIMPLET_POINT_H
#define _SIMPLET_POINT_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

simplet_point_t*
simplet_point_new(double x, double y);

void
simplet_point_free(simplet_point_t *point);

#ifdef __cplusplus
}
#endif

#endif
