#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "map.h"
#include "layer.h"
#include "filter.h"
#include "style.h"
#include "util.h"


#define SIMPLET_SLIPPY_SIZE 256
#define SIMPLET_MERC_LENGTH 40075016.68

simplet_map_t*
simplet_map_new(){
  simplet_map_t *map;
  if(!(map = malloc(sizeof(*map))))
    return NULL;

  if(!(map->layers = simplet_list_new())){
    free(map);
    return NULL;
  }

  map->bounds = NULL;
  map->proj   = NULL;
  map->_ctx   = NULL;
  map->height = 0;
  map->width  = 0;
  map->valid  = MAP_OK;
  return map;
}

void
simplet_map_free(simplet_map_t *map){
  if(map->bounds)
    simplet_bounds_free(map->bounds);

  if(map->layers) {
    map->layers->free = simplet_layer_vfree;
    simplet_list_free(map->layers);
  }

  if(map->_ctx)
    cairo_destroy(map->_ctx);

  if(map->proj)
    OSRRelease(map->proj);

  free(map);
}

int
simplet_map_set_srs(simplet_map_t *map, const char *proj){
  if(map->proj)
    OSRRelease(map->proj);

  if(!(map->proj = OSRNewSpatialReference(NULL)))
    return (map->valid = MAP_ERR);

  if(OSRSetFromUserInput(map->proj, proj) != OGRERR_NONE)
    return (map->valid = MAP_ERR);

  return MAP_OK;
}

void
simplet_map_get_srs(simplet_map_t *map, char **srs){
  OSRExportToProj4(map->proj, srs);
}

int
simplet_map_set_size(simplet_map_t *map, int width, int height){
  map->height = height;
  map->width  = width;
  return MAP_OK;
}

int
simplet_map_set_bounds(simplet_map_t *map, double maxx, double maxy, double minx, double miny){
  if(map->bounds)
    simplet_bounds_free(map->bounds);

  if(!(map->bounds = simplet_bounds_new()))
    return (map->valid = MAP_ERR);

  simplet_bounds_extend(map->bounds, maxx, maxy);
  simplet_bounds_extend(map->bounds, minx, miny);
  return MAP_OK;
}

simplet_layer_t*
simplet_map_add_layer(simplet_map_t *map, const char *datastring){
  simplet_layer_t *layer;
  if(!(layer = simplet_layer_new(datastring))){
    map->valid = MAP_ERR;
    return NULL;
  }

  if(!simplet_list_push(map->layers, layer)){
    map->valid = MAP_ERR;
    simplet_layer_free(layer);
    return NULL;
  }

  return layer;
}

simplet_filter_t*
simplet_map_add_filter(simplet_map_t *map, const char *sqlquery){
  if(!map->layers->tail){
    map->valid = MAP_ERR;
    return NULL;
  }

  simplet_layer_t *layer = map->layers->tail->value;
  if(!layer){
    map->valid = MAP_ERR;
    return NULL;
  }

  simplet_filter_t *filter;
  if(!(filter = simplet_layer_add_filter(layer, sqlquery)))
    return NULL;

  return filter;
}

simplet_style_t *
simplet_map_add_style(simplet_map_t *map, const char *key, const char *arg){
  if(!map->layers->tail){
    map->valid = MAP_ERR;
    return NULL;
  }
  simplet_layer_t *layer = map->layers->tail->value;

  if(!layer){
    map->valid = MAP_ERR;
    return NULL;
  }

  simplet_filter_t *filter = layer->filters->tail->value;

  if(!filter){
    map->valid = MAP_ERR;
    return NULL;
  }

  simplet_style_t *style;
  if(!(style = simplet_filter_add_style(filter, key, arg))){
    map->valid = MAP_ERR;
    return NULL;
  }

  return style;
}

int
simplet_map_is_valid(simplet_map_t *map){
  if(map->valid == MAP_ERR)
    return MAP_ERR;

  if(!map->bounds)
    return MAP_ERR;

  if(!map->proj)
    return MAP_ERR;

  if(!map->height)
    return MAP_ERR;

  if(!map->width)
    return MAP_ERR;

  if(!map->layers->tail)
    return MAP_ERR;

  return MAP_OK;
}


cairo_surface_t *
simplet_map_build_surface(simplet_map_t *map){
  if(simplet_map_is_valid(map) == MAP_ERR)
    return NULL;
  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, map->width, map->height);
  if(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    return NULL;
  cairo_t *ctx = cairo_create(surface);
  map->_ctx = ctx;
  simplet_listiter_t *iter = simplet_get_list_iter(map->layers);
  simplet_layer_t *layer;
  while((layer = simplet_list_next(iter)))
    simplet_layer_process(layer, map);
  return surface;
}

void
simplet_map_close_surface(simplet_map_t *map, cairo_surface_t *surface){
  cairo_destroy(map->_ctx);
  map->_ctx = NULL;
  cairo_surface_destroy(surface);
}

int
simplet_map_render_to_stream(simplet_map_t *map, void *stream,
  cairo_status_t (*cb)(void *closure, const unsigned char *data, unsigned int length)){
  cairo_surface_t *surface;
  if(!(surface = simplet_map_build_surface(map)))
    return (map->valid = MAP_ERR);
  if(cairo_surface_write_to_png_stream(surface, cb, stream) != CAIRO_STATUS_SUCCESS)
    return (map->valid = MAP_ERR);
  simplet_map_close_surface(map, surface);
  return MAP_OK;
}

int
simplet_map_render_to_png(simplet_map_t *map, const char *path){
  cairo_surface_t *surface;
  if(!(surface = simplet_map_build_surface(map)))
    return (map->valid = MAP_ERR);
  if(cairo_surface_write_to_png(surface, path) != CAIRO_STATUS_SUCCESS)
    return (map->valid = MAP_ERR);
  simplet_map_close_surface(map, surface);
  return MAP_OK;
}

int
simplet_map_set_slippy(simplet_map_t *map, double x, double y, double z){
  simplet_map_set_size(map, SIMPLET_SLIPPY_SIZE, SIMPLET_SLIPPY_SIZE);

  if(!simplet_map_set_srs(map, SIMPLET_MERCATOR))
    return (map->valid = MAP_ERR);

  double zfactor, length, origin;

  zfactor = pow(2.0, z);
  length  = SIMPLET_MERC_LENGTH / zfactor;
  origin  = SIMPLET_MERC_LENGTH / 2;

  if(!simplet_map_set_bounds(map, (x + 1) * length - origin,
                                  origin - (y + 1) * length,
                                  x * length - origin,
                                  origin - y * length))
    return (map->valid = MAP_ERR);

  return MAP_OK;
}