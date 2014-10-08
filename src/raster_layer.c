#include "raster_layer.h"
#include "util.h"
#include "error.h"
#include "memory.h"
#include "map.h"

#ifndef __APPLE__
#define __APPLE__ 1
#endif
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#include <OpenGL/OpenGL.h>

#include <gdal.h>
#include <gdal_alg.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

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

  simplet_retain((simplet_retainable_t *)layer);

  return layer;
}

void
simplet_raster_layer_set_resample(simplet_raster_layer_t *layer, simplet_resample_kernel_t kernel) {
  layer->kernel = kernel;
}

simplet_resample_kernel_t
simplet_raster_layer_get_resample(simplet_raster_layer_t *layer) {
  return layer->kernel;
}

void
simplet_raster_layer_free(simplet_raster_layer_t *layer){
  if(simplet_release((simplet_retainable_t *)layer) > 0) return;
  if(layer->error_msg) free(layer->error_msg);
  free(layer->source);
  free(layer);
}

// from: http://www.ipol.im/pub/art/2011/g_lmii/
double
simplet_bilinear(const double value){
  double x = fabs(value);
  if(x < 1) return 1 - x;
  return 0;
}

double
simplet_bicubic(const double value){
  double t = fabs(value);
  double a = -0.5;
  if(t <= 1.0) {
    return (a + 2) * t * t * t - (a + 3) * t * t + 1;
  } else if(t < 2.0) {
    return a * t * t * t - 5 * a * t * t + 8 * a * t - 4 * a;
  }
  return 0;
}

double
simplet_average(const double value) {
  (void) value;
  return 1;
}

double
simplet_lanczos(const double value){
  double w = 2.0;
  if(fabs(value) >= w) return 0.0;
  if(value == 0.0) return 1.0;
  double x = value;
  return sin(M_PI * x) / (M_PI * x) * sin(M_PI * x / w) / (M_PI  * x / w);
}

simplet_status_t
simplet_raster_layer_process(simplet_raster_layer_t *layer, simplet_map_t *map, cairo_t *ctx) {
  // process the map
  int width  = map->width;
  int height = map->height;
  if(layer->resample) { width *= 2; height *= 2; }

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

  // grab WKTs from source and dest
  const char *src_wkt  = GDALGetProjectionRef(source);
  char *dest_wkt;
  OSRExportToWkt(map->proj, &dest_wkt);

  // get a transformer
  void *transform_args = GDALCreateGenImgProjTransformer3(src_wkt, src_t, dest_wkt, dst_t);
  free(dest_wkt);
  if(transform_args == NULL)
    return set_error(layer, SIMPLET_GDAL_ERR, "transform failed");

  double* x_lookup = malloc(width * sizeof(double));
  double* y_lookup = malloc(width * sizeof(double));
  double* z_lookup = malloc(width * sizeof(double));
  int* test = malloc(width * sizeof(int));
  uint32_t *data = malloc(sizeof(uint32_t) * width * height);

  // draw to cairo
  for(int y = 0; y < height; y++){
    // write center of our pixel positions to the destination scanline
    for(int k = 0; k < width; k++){
      x_lookup[k] = k + 0.5;
      y_lookup[k] = y + 0.5;
      z_lookup[k] = 0.0;
    }
    uint32_t *scanline = data + y * width;

    // set an opaque alpha value for RGB images
    if(bands < 4)
      for(int i = 0; i < width; i++)
        scanline[i] = 0xff << 24;

    GDALGenImgProjTransform(transform_args, TRUE, width, x_lookup, y_lookup, z_lookup, test);

    for(int x = 0; x < width; x++) {
      // could not transform the point, skip this pixel
      if(!test[x]) continue;

      // sanity check? From gdalsimplewarp
      if(x_lookup[x] < 0.0 || y_lookup[x] < 0.0) continue;

      // check to see if we are outside of the raster
      if(x_lookup[x] > GDALGetRasterXSize(source)
         || y_lookup[x] > GDALGetRasterYSize(source)) continue;

      for(int band = 1; band <= bands; band++) {
        GByte pixel = 0;
        GDALRasterBandH b = GDALGetRasterBand(source, band);
        GDALRasterIO(b, GF_Read, (int) x_lookup[x], (int) y_lookup[x], 1, 1, &pixel, 1, 1, GDT_Byte, 0, 0);

        // set the pixel to fully transparent if we don't have a pixel value
        int has_no_data = 0;
        double no_data = GDALGetRasterNoDataValue(b, &has_no_data);
        if(has_no_data && no_data == pixel) {
          scanline[x] = 0x00 << 24;
          continue;
        }

        int band_remap[5] = {0, 2, 1, 0, 3};
        scanline[x] |= ((int)pixel) << ((band_remap[band]) * 8);
      }
    }
  }

  if(layer->resample) {
    // on os x
    CGLContextObj context;
    CGLPixelFormatAttribute attributes[5] = {
      kCGLPFAAccelerated,
      kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
      kCGLPFADoubleBuffer,
      (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pix;
    CGLError errorCode;
    GLint num;
    errorCode = CGLChoosePixelFormat(attributes, &pix, &num);
    // add error checking here
    errorCode = CGLCreateContext(pix, NULL, &context);
    // add error checking here
    CGLDestroyPixelFormat(pix);
    errorCode = CGLSetCurrentContext(context);

    // and on linux http://cgit.freedesktop.org/mesa/demos/tree/src/osdemos/osdemo.c

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLfloat vertices[] = {
      -1.0, 1.0, 1.0, 1.0, -1.0, -1.0,
      1.0, 1.0, -1.0, 1.0, -1.0, -1.0
    };

    GLuint vbuf;
    glGenBuffers(1, &vbuf);
    glBindBuffer(GL_ARRAY_BUFFER, vbuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint triangles[] = {
      0, 1, 2,
      2, 1, 3
    };

    GLuint ebuf;
    glGenBuffers(1, &ebuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangles), triangles, GL_STATIC_DRAW);

    const GLchar *vertex =
      "#version 150\n"
      "in vec2 position;"
      "out vec2 coord;"
      "void main(){"
      "  coord = position;"
      "}"
    ;

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex, NULL);
    glCompileShader(vertexShader);

    const GLchar *fragment =
      "#version 150\n"
      "uniform sampler2D tex;"
      "in vec2 coord;"
      "out vec4 color;"
      "void main(){"
      "  color = texture(tex, coord);"
      "}"
    ;

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment, NULL);
    glCompileShader(fragmentShader);

    GLint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glBindFragDataLocation(program, 0, "color");
    glLinkProgram(program);
    glUseProgram(program);

    GLint pos = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);

    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLuint outtex;
    glGenTextures(1, &outtex);
    glBindTexture(GL_TEXTURE_2D, outtex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width/2, height/2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outtex, 0);
    GLuint tex;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);

    glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    width /= 2; height /= 2;
    uint32_t *out = malloc(sizeof(uint32_t) * width * height);
    glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, out);

    free(data);
    data = out;

    CGLSetCurrentContext(NULL);
    CGLDestroyContext(context);
  }

  int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
  cairo_surface_t *surface = cairo_image_surface_create_for_data((unsigned char *) data, CAIRO_FORMAT_ARGB32, width, height, stride);

  if(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    set_error(layer, SIMPLET_CAIRO_ERR, (const char *)cairo_status_to_string(cairo_surface_status(surface)));

  cairo_set_source_surface(ctx, surface, 0, 0);
  cairo_paint(ctx);
  cairo_surface_destroy(surface);

  free(x_lookup);
  free(y_lookup);
  free(z_lookup);
  free(test);
  free(data);
  GDALDestroyGenImgProjTransformer(transform_args);
  GDALClose(source);
  return layer->status;
}
