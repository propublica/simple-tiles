#ifndef _SIMPLET_RASTER_LAYER_H
#define _SIMPLET_RASTER_LAYER_H

#include "types.h"
#include "text.h"
#include "user_data.h"
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

simplet_raster_layer_t*
simplet_raster_layer_new(const char *datastring);

void
simplet_raster_layer_free(simplet_raster_layer_t *layer);

simplet_status_t
simplet_raster_layer_process(simplet_raster_layer_t *layer, simplet_map_t *map, cairo_t *ctx);

void
simplet_raster_layer_set_resample(simplet_raster_layer_t *layer, simplet_kern_t resample);

simplet_kern_t
simplet_raster_layer_get_resample(simplet_raster_layer_t *layer);

double
simplet_bilinear(const double value);

double
simplet_bicubic(const double value);

double
simplet_average(const double value);

double
simplet_lanczos(const double value);

SIMPLET_HAS_USER_DATA_PROTOS(raster_layer)


#ifdef __cplusplus
}
#endif

#endif
