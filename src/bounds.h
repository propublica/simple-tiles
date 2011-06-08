#ifndef _SIMPLET_BOUNDS_H
#define _SIMPLET_BOUNDS_H

#include <gdal/ogr_api.h>
#include <gdal/ogr_srs_api.h>
#include "types.h"
#include "point.h"

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

#ifdef __cplusplus
}
#endif

#endif
