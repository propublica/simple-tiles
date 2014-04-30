#include "raster_layer.h"
#include "gdal_in_mem_warp.h"
#include "util.h"
#include "error.h"
#include "memory.h"
#include "map.h"
#include <gdal.h>
#include <gdal_alg.h>
#include <stdbool.h>


// Add in an error function.
SIMPLET_ERROR_FUNC(raster_layer_t)

simplet_raster_layer_t*
simplet_raster_layer_new(const char *datastring) {
  simplet_raster_layer_t *layer;
  if (!(layer = malloc(sizeof(*layer))))
    return NULL;

  memset(layer, 0, sizeof(*layer));
  layer->source = simplet_copy_string(datastring);
  layer->type   = SIMPLET_RASTER;
  layer->status = SIMPLET_OK;

  // need some checking here
  simplet_retain((simplet_retainable_t *)layer);

  return layer;
}

// Free a layer object, and associated layers.
void
simplet_raster_layer_free(simplet_raster_layer_t *layer){
  if(simplet_release((simplet_retainable_t *)layer) > 0) return;
  if(layer->error_msg) free(layer->error_msg);
  free(layer->source);
  free(layer);
}

simplet_status_t
simplet_raster_layer_process(simplet_raster_layer_t *layer, simplet_map_t *map, cairo_t *ctx) {
  // process the map
  int width  = map->width;
  int height = map->height;

  GDALDatasetH source;

  source = GDALOpen(layer->source, GA_ReadOnly);
  if(source == NULL)
    return set_error(layer, SIMPLET_GDAL_ERR, "error opening raster source");

  int bands = GDALGetRasterCount(source);
  if(bands > 4) bands = 4;

  // create geotransform
  double src_t[6];
  if(GDALGetGeoTransform(source, src_t) != CE_None)
    return set_error(layer, SIMPLET_GDAL_ERR, "can't get geotransform on dataset");

  double dst_t[6];
  cairo_matrix_t mat;
  simplet_map_init_matrix(map, &mat);
  cairo_matrix_invert(&mat);
  dst_t[0] = mat.x0;
  dst_t[1] = mat.xx;
  dst_t[2] = mat.xy;
  dst_t[3] = mat.y0;
  dst_t[4] = mat.yx;
  dst_t[5] = mat.yy;

  // grab WKTs from source and dest
  const char *src_wkt  = GDALGetProjectionRef(source);
  char *dest_wkt;
  OSRExportToWkt(map->proj, &dest_wkt);

  // get a transformer
  void *transform_args = GDALCreateGenImgProjTransformer3(src_wkt, src_t, dest_wkt, dst_t);
  free(dest_wkt);
  if(transform_args == NULL)
    return set_error(layer, SIMPLET_GDAL_ERR, "transform failed");

  double* x = calloc(width, sizeof(double));
  double* y = calloc(width, sizeof(double));
  double* z = calloc(width, sizeof(double));
  int* test = calloc(width, sizeof(int));

  // draw to cairo
  for(int i = 0; i < height; i++){
    // write pixel positions to the destination scanline
    for(int k = 0; k < width; k++){
      x[k] = k + 0.5;
      y[k] = i + 0.5;
      z[k] = 0.0;
    }

    // initialize arrays
    GDALGenImgProjTransform(transform_args, TRUE, width, x, y, z, test);
    for(int j = 0; j < width; i++) {
      // could not transform the point, skip this pixel
      if(!test[j]) continue;
      // sanity check? From gdalsimplewarp
      if(x[j] < 0.0 || y[j] < 0.0) continue;

      int src_off = x[j] + y[j] * width;
      for(int band = 0; band < bands; band++) {
        // write to cairo
        // cairo[band][j] = gdal[band][src_off];
      }
    }
  }

  free(x);
  free(y);
  free(z);
  free(test);
  GDALDestroyGenImgProjTransformer(transform_args);
  return SIMPLET_OK;
}
