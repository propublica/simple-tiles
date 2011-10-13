#ifndef _SIMPLET_BOUNDS_H
#define _SIMPLET_BOUNDS_H

#include <ogr_api.h>
#include <ogr_srs_api.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

simplet_bounds_t*
simplet_bounds_new();

void
simplet_bounds_extend(simplet_bounds_t *bounds, double x, double y);

OGRGeometryH
simplet_bounds_to_ogr(simplet_bounds_t *bounds, OGRSpatialReferenceH proj);

simplet_bounds_t*
simplet_bounds_from_ogr(OGRGeometryH geom);

void
simplet_bounds_free(simplet_bounds_t *bounds);

simplet_status_t
simplet_bounds_to_wkt(simplet_bounds_t *bounds, char **wkt);

simplet_bounds_t*
simplet_bounds_reproject(simplet_bounds_t* bounds, const char *from, const char *to);

#ifdef __cplusplus
}
#endif

#endif
