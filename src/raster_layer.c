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
simplet_raster_layer_coords_to_pixels(GDALDatasetH dataset, double points[4], simplet_map_t* map) {
  double gt[6];
  if( GDALGetGeoTransform(dataset, gt) == CE_None ) {
    points[0] = ((map->bounds->se.x - gt[0]) / gt[1]);
    points[1] = ((map->bounds->se.y - gt[3]) / gt[5]);
    points[2] = ((map->bounds->nw.x - gt[0]) / gt[1]);
    points[3] = ((map->bounds->nw.y - gt[3]) / gt[5]);
    return SIMPLET_OK;
  } else {
    return SIMPLET_ERR;
  }
}

char*
simplet_raster_layer_get_scanline(GDALDatasetH dataset, double coords[4], int width, int heightOffset) {
  // allocate buffer
  char *scanline = malloc(sizeof(char) * width * 4);
  if(scanline == NULL) return NULL;

  // zero out the buffer
  memset(scanline, 0, sizeof(char) * width * 4);

  for(int i = 1; i <= GDALGetRasterCount(dataset); i++) {
    GDALRasterIO(GDALGetRasterBand(dataset, i), GF_Read, coords[0], coords[3] + heightOffset, width, 1,
                            // For some reason, GDAL goes BGR
                 scanline + (GDALGetRasterCount(dataset) - i - 1), width, 1, GDT_Byte, // offset the pointer based on whether we are reading red green or blue
                 4, 0 ); // 4 is important because you want the scanlines buffer to be 0,r,g,b,01,r1,g1,b1 for CAIRO_FORMAT_RGB24
  }
  return scanline;
}

void
simplet_raster_layer_set_cairo(cairo_t *ctx, int width, int height, unsigned char* image) {
  int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
  cairo_surface_t *surface = cairo_image_surface_create_for_data(image, CAIRO_FORMAT_ARGB32, width, height, stride);
  cairo_set_source_surface(ctx, surface, 0, 0);
  cairo_paint(ctx);
  cairo_surface_destroy(surface);
  free(image);
}


unsigned char*
simplet_raster_layer_crop_image(GDALDatasetH dataset, double coords[4]) {
  int height = coords[2] - coords[0];
  int width  = coords[1] - coords[3];
  unsigned char *image = malloc(height * sizeof(char) * width * 4);

  for (int i = 0; i < height; i++) {
    char *scanline = simplet_raster_layer_get_scanline(dataset, coords, width, i);
    // memcpy(image + i * nXSize * 3, nXSize * 3, scanline);
    memcpy(image + i * width * 4, scanline, width * 4);
    free(scanline);
  }
  printf("sizeof image: %lu\n", sizeof(image));
  return image;
}


simplet_status_t
simplet_raster_layer_process(simplet_raster_layer_t *layer, simplet_map_t *map, simplet_lithograph_t *litho, cairo_t *ctx) {
  GDALAllRegister();
  GDALDatasetH source;
  source = GDALOpen(layer->source, GA_ReadOnly);
  if (source == NULL) {
    return set_error(layer, SIMPLET_GDAL_ERR, "error opening raster source");
  }
  if (GDALGetRasterCount(source) < 4) {
    return set_error(layer, SIMPLET_GDAL_ERR, "raster layer must have 4 bands");
  }

  // process the map
  int width  = map->bounds->nw.x - map->bounds->se.x;
  int height = map->bounds->se.y - map->bounds->nw.y;
  double coords[4];
  if(simplet_raster_layer_coords_to_pixels(source, coords, map) == FALSE) return SIMPLET_ERR;
  unsigned char *image = simplet_raster_layer_crop_image(source, coords);
  simplet_raster_layer_set_cairo(ctx, width, height, image);
  return SIMPLET_OK;
}
