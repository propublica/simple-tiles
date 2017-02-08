#include <gdal.h>
#include <gdal_alg.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "raster_layer.h"
#include "util.h"
#include "error.h"
#include "memory.h"
#include "map.h"

// Add in an error function.
SIMPLET_ERROR_FUNC(raster_layer_t)

SIMPLET_HAS_USER_DATA(raster_layer)

simplet_raster_layer_t *simplet_raster_layer_new(const char *datastring) {
  simplet_raster_layer_t *layer;
  if (!(layer = malloc(sizeof(*layer)))) return NULL;

  memset(layer, 0, sizeof(*layer));
  layer->source = simplet_copy_string(datastring);
  layer->type = SIMPLET_RASTER;
  layer->status = SIMPLET_OK;

  simplet_retain((simplet_retainable_t *)layer);

  return layer;
}

void simplet_raster_layer_set_resample(simplet_raster_layer_t *layer,
                                       simplet_kern_t resample) {
  layer->resample = resample;
}

simplet_kern_t simplet_raster_layer_get_resample(
    simplet_raster_layer_t *layer) {
  return layer->resample;
}

void simplet_raster_layer_free(simplet_raster_layer_t *layer) {
  if (simplet_release((simplet_retainable_t *)layer) > 0) return;
  if (layer->error_msg) free(layer->error_msg);
  free(layer->source);
  free(layer);
}

static double sinc(double x) {
  if (x == 0.0) return 1.0;
  return sin(M_PI * x) / (M_PI * x);
}

simplet_status_t simplet_raster_layer_process(simplet_raster_layer_t *layer,
                                              simplet_map_t *map,
                                              cairo_t *ctx) {
  // process the map
  int width = map->width;
  int height = map->height;

  GDALDatasetH source = GDALOpen(layer->source, GA_ReadOnly);
  if (source == NULL)
    return set_error(layer, SIMPLET_GDAL_ERR, "error opening raster source");

  int bands = GDALGetRasterCount(source);
  if (bands > 4) bands = 4;

  // create geotransform
  double src_t[6];
  if (GDALGetGeoTransform(source, src_t) != CE_None)
    return set_error(layer, SIMPLET_GDAL_ERR,
                     "can't get geotransform on dataset");

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
  const char *src_wkt = GDALGetProjectionRef(source);
  char *dest_wkt;
  OSRExportToWkt(map->proj, &dest_wkt);

  // get a transformer
  void *transform_args =
      GDALCreateGenImgProjTransformer3(src_wkt, src_t, dest_wkt, dst_t);
  free(dest_wkt);
  if (transform_args == NULL)
    return set_error(layer, SIMPLET_GDAL_ERR, "transform failed");

  double *x_lookup = malloc(width * sizeof(double));
  double *y_lookup = malloc(width * sizeof(double));
  double *z_lookup = malloc(width * sizeof(double));
  int *test = malloc(width * sizeof(int));
  uint32_t *data = malloc(sizeof(uint32_t) * width * height);
  memset(data, 0, sizeof(uint32_t) * width * height);

  int kernel_size;
  double *kernel;

  switch (layer->resample) {
    case SIMPLET_NEAREST:
      kernel_size = 1;
      kernel = calloc(kernel_size, sizeof(*kernel));
      kernel[0] = 1;
      break;
    case SIMPLET_BILINEAR:
      kernel_size = 3;
      kernel = calloc(kernel_size, sizeof(*kernel));
      kernel[0] = 0.25;
      kernel[1] = 0.5;
      kernel[2] = 0.25;
      break;
    case SIMPLET_LANCZOS:
      kernel_size = 9;
      double tot = 0;
      kernel = calloc(kernel_size, sizeof(double));
      for (int i = 0; i < kernel_size; i++) {
        double x = (double)i - kernel_size / 2.0 + 0.5;
        // the divided by three is this one weird trick from here:
        // http://cbloomrants.blogspot.com/2011/03/03-24-11-image-filters-and-gradients.html
        kernel[i] = sinc(x / 3) * sinc(x / (kernel_size / 2));
        tot += kernel[i];
      }
      for (int i = 0; i < kernel_size; i++) kernel[i] /= tot;
      break;
    default:
      return set_error(layer, SIMPLET_ERR, "unknown resample kernel");
  }

  // draw to cairo
  for (int y = 0; y < height; y++) {
    // write center of our pixel positions to the destination scanline
    for (int k = 0; k < width; k++) {
      x_lookup[k] = k + 0.5;
      y_lookup[k] = y + 0.5;
      z_lookup[k] = 0.0;
    }
    uint32_t *scanline = data + y * width;

    // set an opaque alpha value for RGB images
    if (bands < 4)
      for (int i = 0; i < width; i++) scanline[i] = 0xff << 24;

    GDALGenImgProjTransform(transform_args, TRUE, width, x_lookup, y_lookup,
                            z_lookup, test);

    for (int x = 0; x < width; x++) {
      // could not transform the point, skip this pixel
      if (!test[x]) {
        scanline[x] = 0x00 << 24;
        continue;
      }

      // sanity check? From gdalsimplewarp
      if (x_lookup[x] < 0.0 || y_lookup[x] < 0.0) {
        scanline[x] = 0x00 << 24;
        continue;
      }

      // check to see if we are outside of the raster
      if (x_lookup[x] > GDALGetRasterXSize(source) ||
          y_lookup[x] > GDALGetRasterYSize(source)) {
        scanline[x] = 0x00 << 24;
        continue;
      }

      // clean up needed here
      for (int band = 1; band <= bands; band++) {
        double pixel = 0;
        GByte window[kernel_size * kernel_size];
        memset(window, 0, kernel_size * kernel_size);
        GDALRasterBandH b = GDALGetRasterBand(source, band);

        CPLErr err = GDALRasterIO(b, GF_Read, (int)x_lookup[x] - kernel_size / 2,
                                  (int)y_lookup[x] - kernel_size / 2,
                                  kernel_size, kernel_size,
                                  window,
                                  kernel_size, kernel_size,
                                  GDT_Byte, 0, 0);

        for (int kx = 0; kx < kernel_size; kx++) {
          for (int ky = 0; ky < kernel_size; ky++) {
            GByte px = window[kx * kernel_size + ky];
            pixel += px * kernel[kx] * kernel[ky];
          }
        }

        // set the pixel to fully transparent if we don't have a pixel value
        int has_no_data = 0;
        double no_data = GDALGetRasterNoDataValue(b, &has_no_data);
        if (has_no_data && no_data == window[(kernel_size * kernel_size) / 2]) {
          scanline[x] = 0x00 << 24;
          continue;
        }

        int band_remap[5] = {0, 2, 1, 0, 3};
        scanline[x] |= ((int)fmax(0, fmin(255, pixel)))
                       << ((band_remap[band]) * 8);
      }
    }
  }

  int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, map->width);
  cairo_surface_t *surface = cairo_image_surface_create_for_data(
      (unsigned char *)data, CAIRO_FORMAT_ARGB32, map->width, map->height,
      stride);

  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    set_error(layer, SIMPLET_CAIRO_ERR, (const char *)cairo_status_to_string(
                                            cairo_surface_status(surface)));

  cairo_set_source_surface(ctx, surface, 0, 0);
  cairo_paint(ctx);
  cairo_surface_destroy(surface);

  free(x_lookup);
  free(y_lookup);
  free(z_lookup);
  free(test);
  free(data);
  free(kernel);
  GDALDestroyGenImgProjTransformer(transform_args);
  GDALClose(source);
  return layer->status;
}
