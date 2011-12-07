#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "init.h"
#include "error.h"
#include "map.h"
#include "layer.h"
#include "filter.h"
#include "style.h"
#include "util.h"
#include "bounds.h"
#include "text.h"

#define SIMPLET_SLIPPY_SIZE 256
#define SIMPLET_MERC_LENGTH 40075016.68

simplet_map_t*
simplet_map_new(){
  simplet_init();
  simplet_map_t *map;
  if(!(map = malloc(sizeof(*map))))
    return NULL;

  memset(map, 0, sizeof(*map));

  if(!(map->layers = simplet_list_new())){
    free(map);
    return NULL;
  }

  if(!(map->bounds = simplet_bounds_new())){
    simplet_list_free(map->layers);
    free(map);
    return NULL;
  }

  map->error.status = SIMPLET_OK;

  return map;
}

void
simplet_map_free(simplet_map_t *map){
  if(map->bounds)
    simplet_bounds_free(map->bounds);

  if(map->layers) {
    simplet_list_set_item_free(map->layers, simplet_layer_vfree);
    simplet_list_free(map->layers);
  }

  if(map->proj)
    OSRRelease(map->proj);

  if(map->bgcolor)
    free(map->bgcolor);

  free(map);
}

SIMPLET_ERROR_FUNC(map_t)

simplet_status_t
simplet_map_set_srs(simplet_map_t *map, const char *proj){
  if(map->proj) {
    if(map->bounds) {
      simplet_bounds_t *tmp = map->bounds;
      char *s;
      simplet_map_get_srs(map, &s);
      map->bounds = simplet_bounds_reproject(map->bounds, (const char *) s, (const char *) proj);
      free(s);
      simplet_bounds_free(tmp);
    }
    OSRRelease(map->proj);
  }

  if(!(map->proj = OSRNewSpatialReference(NULL)))
    return set_error(map, SIMPLET_OGR_ERR, "could not assign spatial ref");

  if(OSRSetFromUserInput(map->proj, proj) != OGRERR_NONE)
    return set_error(map, SIMPLET_OGR_ERR, "bad projection string");

  return SIMPLET_OK;
}

void
simplet_map_set_buffer(simplet_map_t *map, double buffer){
  map->buffer = buffer;
}

double
simplet_map_get_buffer(simplet_map_t *map){
  return map->buffer;
}

void
simplet_map_get_srs(simplet_map_t *map, char **srs){
  OSRExportToProj4(map->proj, srs);
}

void
simplet_map_init_matrix(simplet_map_t *map, cairo_matrix_t *mat){
  cairo_matrix_init(mat, 1, 0, 0, -1, 0, 0);
  cairo_matrix_translate(mat, 0, map->height * -1.0);
  cairo_matrix_scale(mat, map->width / map->bounds->width, map->width / map->bounds->width);
  cairo_matrix_translate(mat, -map->bounds->nw.x, -map->bounds->se.y);
}

simplet_status_t
simplet_map_set_size(simplet_map_t *map, int width, int height){
  map->height = height;
  map->width  = width;
  return SIMPLET_OK;
}

simplet_status_t
simplet_map_set_bgcolor(simplet_map_t *map, const char *str){
  free(map->bgcolor);
  if((map->bgcolor = simplet_copy_string(str)))
    return SIMPLET_OK;
  return set_error(map, SIMPLET_OOM, "couldn't copy bgcolor");
}

void
simplet_map_get_bgcolor(simplet_map_t *map, char **str){
  *str = simplet_copy_string(map->bgcolor);
}

simplet_status_t
simplet_map_set_bounds(simplet_map_t *map, double maxx, double maxy, double minx, double miny){
  if(map->bounds)
    simplet_bounds_free(map->bounds);

  if(!(map->bounds = simplet_bounds_new()))
    return set_error(map, SIMPLET_OOM, "couldn't create bounds");

  simplet_bounds_extend(map->bounds, maxx, maxy);
  simplet_bounds_extend(map->bounds, minx, miny);
  return SIMPLET_OK;
}

simplet_status_t
simplet_map_set_slippy(simplet_map_t *map, unsigned int x, unsigned int y, unsigned int z){
  simplet_map_set_size(map, SIMPLET_SLIPPY_SIZE, SIMPLET_SLIPPY_SIZE);

  if(!simplet_map_set_srs(map, SIMPLET_MERCATOR))
    return set_error(map, SIMPLET_OGR_ERR, "couldn't set slippy projection");

  double zfactor, length, origin;
  zfactor = pow(2.0, z);
  length  = SIMPLET_MERC_LENGTH / zfactor;
  origin  = SIMPLET_MERC_LENGTH / 2;

  if(!simplet_map_set_bounds(map, (x + 1) * length - origin,
                                  origin - (y + 1) * length,
                                  x * length - origin,
                                  origin - y * length))
    return simplet_error((simplet_errorable_t *) map, SIMPLET_OOM, "out of memory setting bounds");

  return SIMPLET_OK;
}

simplet_layer_t*
simplet_map_add_layer(simplet_map_t *map, const char *datastring){
  simplet_layer_t *layer;
  if(!(layer = simplet_layer_new(datastring))){
    set_error(map, SIMPLET_OOM, "couldn't create a layer");
    return NULL;
  }

  if(!simplet_list_push(map->layers, layer)){
    simplet_layer_free(layer);
    set_error(map, SIMPLET_OOM, "couldn't add any more layers");
    return NULL;
  }

  return layer;
}


simplet_status_t
simplet_map_get_status(simplet_map_t *map){
  return map->error.status;
}

const char*
simplet_map_status_to_string(simplet_map_t *map){
  return (const char*) map->error.msg;
}

simplet_status_t
simplet_map_is_valid(simplet_map_t *map){
  if(!map->error.status == SIMPLET_OK)
    return SIMPLET_ERR;

  if(!map->bounds)
    return SIMPLET_ERR;

  if(!map->proj)
    return SIMPLET_ERR;

  if(!map->height)
    return SIMPLET_ERR;

  if(!map->width)
    return SIMPLET_ERR;

  if(!simplet_list_head(map->layers))
    return SIMPLET_ERR;

  return SIMPLET_OK;
}

static cairo_surface_t *
build_surface(simplet_map_t *map){
  if(simplet_map_is_valid(map) == SIMPLET_ERR)
    return NULL;

  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
      map->width, map->height);

  if(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    return NULL;

  cairo_t *ctx = cairo_create(surface);

  if(map->bgcolor) simplet_style_paint(ctx, map->bgcolor);

  simplet_listiter_t *iter = simplet_get_list_iter(map->layers);
  simplet_layer_t *layer;
  simplet_status_t err;

  cairo_t *litho_ctx = cairo_create(surface);
  simplet_lithograph_t *litho = simplet_lithograph_new(litho_ctx);

  // defaults
  simplet_style_line_join(litho_ctx, "round");

  while((layer = simplet_list_next(iter))){
    err = simplet_layer_process(layer, map, litho, ctx);
    if(err != SIMPLET_OK) {
      simplet_list_iter_free(iter);
      set_error(map, err, "error in rendering");
      break;
    }
  }

  simplet_lithograph_free(litho);
  cairo_destroy(ctx);
  cairo_destroy(litho_ctx);
  return surface;
}

static void
close_surface(cairo_surface_t *surface){
  cairo_surface_destroy(surface);
}

void
simplet_map_render_to_stream(simplet_map_t *map, void *stream,
  cairo_status_t (*cb)(void *closure, const unsigned char *data, unsigned int length)){
  cairo_surface_t *surface;
  if(!(surface = build_surface(map))) return;

  if(cairo_surface_write_to_png_stream(surface, cb, stream) != CAIRO_STATUS_SUCCESS)
    set_error(map, SIMPLET_CAIRO_ERR, cairo_status_to_string(cairo_surface_status(surface)));

  close_surface(surface);
}

void
simplet_map_render_to_png(simplet_map_t *map, const char *path){
  cairo_surface_t *surface;
  if(!(surface = build_surface(map))) return;

  if(cairo_surface_write_to_png(surface, path) != CAIRO_STATUS_SUCCESS)
    set_error(map, SIMPLET_CAIRO_ERR, cairo_status_to_string(cairo_surface_status(surface)));

  close_surface(surface);
}

