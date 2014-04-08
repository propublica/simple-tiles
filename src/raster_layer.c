#include "raster_layer.h"
#include "util.h"
#include "error.h"
#include <gdal.h>
#include <cpl_error.h>
#include <cpl_conv.h> /* for CPLMalloc() */
#include "memory.h"

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
simplet_raster_layer_process(simplet_raster_layer_t *layer, simplet_map_t *map, simplet_lithograph_t *litho, cairo_t *ctx) {
  // process the map
  int width  = map->width;
  int height = map->height;
  GDALAllRegister();
  GDALDatasetH source;
  GDALDatasetH dst_mem;

  source = GDALOpen(layer->source, GA_ReadOnly);
  if (source == NULL) {
    return set_error(layer, SIMPLET_GDAL_ERR, "error opening raster source");
  }
  if (GDALGetRasterCount(source) < 4) {
    return set_error(layer, SIMPLET_GDAL_ERR, "raster layer must have 4 bands");
  }
  char *dst = malloc(sizeof(char) * width * height * 4);
  char mem_init_str[255];
  snprintf(mem_init_str, 255, "MEM:::DATAPOINTER=%d,PIXELS=%i,LINES=%i,BANDS=3,DATATYPE=Byte", dst, width, height);
  printf("%s\n", mem_init_str);
  dst_mem = GDALOpen(mem_init_str, GA_Update);
  if (dst_mem == NULL) {
    return set_error(layer, SIMPLET_GDAL_ERR, "error creating in-memory raster");
  }

  return SIMPLET_OK;
}
