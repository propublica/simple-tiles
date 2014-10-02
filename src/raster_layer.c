#include "raster_layer.h"
#include "util.h"
#include "error.h"
#include "memory.h"
#include "map.h"
#include <gdal.h>
#include <gdal_alg.h>
#include <gdalwarper.h>
#include <stdbool.h>
#include <stdint.h>

// Add in an error function.
SIMPLET_ERROR_FUNC(raster_layer_t)

SIMPLET_HAS_USER_DATA(raster_layer)

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

  GDALDatasetH source = GDALOpen(layer->source, GA_ReadOnly);
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

  GDALDriverH mem_driver = GDALGetDriverByName("MEM");
  if(mem_driver == NULL)
    return set_error(layer, SIMPLET_GDAL_ERR, "couldn't get memdriver");

  GDALDatasetH out = GDALCreate(mem_driver, "simple_tiles_memory_dataset",
                                width, height, bands, GDT_Byte, NULL);
  if(out == NULL)
    return set_error(layer, SIMPLET_GDAL_ERR, "couldn't allocate memory for image");

  GDALSetGeoTransform(out, dst_t);
  GDALWarpOptions *warp_opts = GDALCreateWarpOptions();
  warp_opts->hSrcDS = source;
  warp_opts->hDstDS = out;
  warp_opts->nBandCount = bands;
  warp_opts->panSrcBands = malloc(sizeof(int) * bands);
  warp_opts->panDstBands = malloc(sizeof(int) * bands);
  for(int i = 0; i < bands; i++)
    warp_opts->panSrcBands[i] = warp_opts->panDstBands[i] = 1;

  // grab WKTs from source and dest
  const char *src_wkt  = GDALGetProjectionRef(source);
  char *dest_wkt;
  OSRExportToWkt(map->proj, &dest_wkt);
  GDALSetProjection(out, dest_wkt);

  // get a transformer
  warp_opts->pTransformerArg = GDALCreateGenImgProjTransformer(source, src_wkt,
                                                               out, dest_wkt, FALSE,
                                                               0.0, 1);
  free(dest_wkt);
  if(warp_opts->pTransformerArg == NULL) {
    set_error(layer, SIMPLET_GDAL_ERR, "transform failed");
    goto transformer_fail;
  }
  warp_opts->pfnTransformer = GDALGenImgProjTransform;

  GDALWarpOperationH warper = GDALCreateWarpOperation(warp_opts);
  if(warper == NULL){
    set_error(layer, SIMPLET_GDAL_ERR, "no warper");
    goto warper_fail;
  }

  CPLErr err = GDALChunkAndWarpMulti(warper, 0, 0, width, height);
  if(err != CPLE_None) {
    set_error(layer, SIMPLET_GDAL_ERR, "warp failed");
    goto warper_fail;
  }

  uint32_t *scanline = malloc(sizeof(uint32_t) * width);

  // draw to cairo
  for(int y = 0; y < height; y++){
    memset(scanline, 0, sizeof(uint32_t) * width);

    // set an opaque alpha value for RGB images
    if(bands < 4)
      for(int i = 0; i < width; i++)
        scanline[i] = 0xff << 24;

    for(int x = 0; x < width; x++) {
      for(int band = 1; band <= bands; band++) {
        GByte pixel = 0;
        GDALRasterBandH b = GDALGetRasterBand(out, band);
        GDALRasterIO(b, GF_Read, x, y, 1, 1, &pixel, 1, 1, GDT_Byte, 0, 0);

        // set the pixel to fully transparent if we don't have a value
        int has_no_data = 0;
        double no_data = GDALGetRasterNoDataValue(b, &has_no_data);
        if(has_no_data && no_data == pixel) {
          scanline[x] = 0x00 << 24;
          continue;
        }

        int band_remap[5] = {0, 2, 1, 0, 3};
        scanline[x] |= pixel << ((band_remap[band]) * 8);
      }
    }

    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
    cairo_surface_t *surface = cairo_image_surface_create_for_data((unsigned char *) scanline, CAIRO_FORMAT_ARGB32, width, 1, stride);

    if(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
      set_error(layer, SIMPLET_CAIRO_ERR, (const char *)cairo_status_to_string(cairo_surface_status(surface)));

    cairo_set_source_surface(ctx, surface, 0, y);
    cairo_paint(ctx);
    cairo_surface_destroy(surface);
  }

  free(scanline);
warper_fail:
  GDALDestroyGenImgProjTransformer(warp_opts->pTransformerArg);
transformer_fail:
  GDALClose(source);
  GDALClose(out);
  GDALDestroyWarpOptions(warp_opts);
  return layer->status;
}
