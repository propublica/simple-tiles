#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "init.h"
#include "error.h"
#include "map.h"
#include "layer.h"
#include "vector_layer.h"
#include "raster_layer.h"
#include "query.h"
#include "style.h"
#include "util.h"
#include "bounds.h"
#include "text.h"
#include "memory.h"

// Output size of a slippy tile.
#define SIMPLET_SLIPPY_SIZE 256

// Size of the earth in mercator meters.
#define SIMPLET_MERC_LENGTH 40075016.68

// Add user_data methods to simplet_map_t.
SIMPLET_HAS_USER_DATA(map)

// Create and return a new simplet_map_t.
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

  map->status = SIMPLET_OK;

  simplet_retain((simplet_retainable_t *)map);
  return map;
}

// Free the memory associated with a simplet_map_t.
void
simplet_map_free(simplet_map_t *map){
  if(simplet_release((simplet_retainable_t *)map) > 0) return;

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

  if(map->error_msg)
    free(map->error_msg);

  free(map);
}

// Add error reporting to simplet_map_t. Macro defined in <b>error.h</b>
SIMPLET_ERROR_FUNC(map_t)

// Set the projection on the map.
simplet_status_t
simplet_map_set_srs(simplet_map_t *map, const char *proj){
  // If this map has a projection and bounds already,
  // it needs to reproject the bounds to the new srs.
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

// Set the overprinting buffer on the map.
void
simplet_map_set_buffer(simplet_map_t *map, double buffer){
  map->buffer = buffer;
}

// Return the current overprinting buffer on the map.
double
simplet_map_get_buffer(simplet_map_t *map){
  return map->buffer;
}

// Store the proj4 string representation of the map in srs
void
simplet_map_get_srs(simplet_map_t *map, char **srs){
  OSRExportToProj4(map->proj, srs);
}

// Initialize the transformation matrix for transforming data source coordinates
// into cairo coordinates.
void
simplet_map_init_matrix(simplet_map_t *map, cairo_matrix_t *mat){
  cairo_matrix_init(mat, 1, 0, 0, -1, 0, 0);
  cairo_matrix_translate(mat, 0, map->height * -1.0);
  cairo_matrix_scale(mat, map->width / map->bounds->width, map->width / map->bounds->width);
  cairo_matrix_translate(mat, -map->bounds->nw.x, -map->bounds->se.y);
}

// Set the width and height of the map.
simplet_status_t
simplet_map_set_size(simplet_map_t *map, unsigned int width, unsigned int height){
  simplet_map_set_height(map, height);
  simplet_map_set_width(map, width);
  return SIMPLET_OK;
}

// Get the width of the map
unsigned int
simplet_map_get_width(simplet_map_t *map){
  return map->width;
}

// Get the height of the map
unsigned int
simplet_map_get_height(simplet_map_t *map){
  return map->height;
}

// Set the width of the map
simplet_status_t
simplet_map_set_width(simplet_map_t *map, unsigned int width){
  map->width = width;
  return SIMPLET_OK;
}

// Set the height of the map
simplet_status_t
simplet_map_set_height(simplet_map_t *map, unsigned int height){
  map->height = height;
  return SIMPLET_OK;
}

// Set the background color of the map to a copy of str.
simplet_status_t
simplet_map_set_bgcolor(simplet_map_t *map, const char *str){
  free(map->bgcolor);
  if((map->bgcolor = simplet_copy_string(str)))
    return SIMPLET_OK;
  return set_error(map, SIMPLET_OOM, "couldn't copy bgcolor");
}

// Return a copy of the map's background color.
void
simplet_map_get_bgcolor(simplet_map_t *map, char **str){
  *str = simplet_copy_string(map->bgcolor);
}

// Set the bounds of the map.
simplet_status_t
simplet_map_set_bounds(simplet_map_t *map, double maxx, double maxy, double minx, double miny){
  simplet_bounds_free(map->bounds);

  if(!(map->bounds = simplet_bounds_new()))
    return set_error(map, SIMPLET_OOM, "couldn't create bounds");

  simplet_bounds_extend(map->bounds, maxx, maxy);
  simplet_bounds_extend(map->bounds, minx, miny);
  return SIMPLET_OK;
}

// Sets the bounds and correct size for a map tile, uses
// [tile coordinates](http://code.google.com/apis/maps/documentation/javascript/maptypes.html#CustomMapTypes)
simplet_status_t
simplet_map_set_slippy(simplet_map_t *map, unsigned int x, unsigned int y, unsigned int z){
  simplet_map_set_size(map, SIMPLET_SLIPPY_SIZE, SIMPLET_SLIPPY_SIZE);

  if(!(simplet_map_set_srs(map, SIMPLET_MERCATOR) == SIMPLET_OK))
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

static void*
add_layer(simplet_map_t *map, simplet_layer_t *layer){
  if(!simplet_list_push(map->layers, layer)){
    simplet_layer_vfree((void*) layer);
    set_error(map, SIMPLET_OOM, "couldn't add any more layers");
    return NULL;
  }

  return layer;
}

// Add a new child layer to the map
simplet_vector_layer_t*
simplet_map_add_vector_layer(simplet_map_t *map, const char *datastring){
  simplet_vector_layer_t *layer;
  if(!(layer = simplet_vector_layer_new(datastring))){
    set_error(map, SIMPLET_OOM, "couldn't create a vector layer");
    return NULL;
  }

  return add_layer(map, (simplet_layer_t *) layer);
}

simplet_raster_layer_t*
simplet_map_add_raster_layer(simplet_map_t *map, const char *datastring) {
  simplet_raster_layer_t *layer;
  if(!(layer = simplet_raster_layer_new(datastring))){
    set_error(map, SIMPLET_OOM, "couldn't create a raster layer");
    return NULL;
  }

  return add_layer(map, (simplet_layer_t *) layer);
}

// Add a previously initialized layer to the map.
simplet_layer_t*
simplet_map_add_layer_directly(simplet_map_t *map, simplet_layer_t *layer){
  if(!simplet_list_push(map->layers, layer)) return NULL;
  return layer;
}

// Check the error status of the map.
simplet_status_t
simplet_map_get_status(simplet_map_t *map){
  return map->status;
}

// Return a human readable reference to the error message stored on the map.
const char*
simplet_map_status_to_string(simplet_map_t *map){
  return (const char*) map->error_msg;
}

// Check if the map is valid for rendering
simplet_status_t
simplet_map_is_valid(simplet_map_t *map){
  // Does it have a previously set error.
  if(!(map->status == SIMPLET_OK))
    return SIMPLET_ERR;

  // Does it have a bounds?
  if(!map->bounds)
    return SIMPLET_ERR;

  // Does it have a projection?
  if(!map->proj)
    return SIMPLET_ERR;

  // Does it have a height?
  if(!map->height)
    return SIMPLET_ERR;

  // ...and width?
  if(!map->width)
    return SIMPLET_ERR;

  // Does it have at least one layer?
  if(!simplet_list_head(map->layers))
    return SIMPLET_ERR;

  // Then we are good to go.
  return SIMPLET_OK;
}

// Build a rendering context to draw the map on.
cairo_surface_t *
simplet_map_build_surface(simplet_map_t *map){
  // Check if the map is valid.
  if(simplet_map_is_valid(map) == SIMPLET_ERR)
    return NULL;

  // Create a cairo surface to draw on.
  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
      map->width, map->height);

  if(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    return NULL;

  cairo_t *ctx = cairo_create(surface);

  // Paint the background color.
  if(map->bgcolor) simplet_style_paint(ctx, map->bgcolor);

  simplet_listiter_t *iter = simplet_get_list_iter(map->layers);

  cairo_t *litho_ctx = cairo_create(surface);

  // Set up a map-wide text structure.
  simplet_lithograph_t *litho = simplet_lithograph_new(litho_ctx);

  // Set a sensible default.
  simplet_style_line_join(litho_ctx, "round");

  // Iterate through and draw all the layers on the cairo context.
  simplet_layer_t *layer;
  while((layer = simplet_list_next(iter))){
    simplet_status_t err = SIMPLET_OK;
    if (layer->type == SIMPLET_VECTOR) {
      err = simplet_vector_layer_process((simplet_vector_layer_t*) layer, map, litho, ctx);
    } else if (layer->type == SIMPLET_RASTER) {
      err = simplet_raster_layer_process((simplet_raster_layer_t*) layer, map, ctx);
    }

    if(err != SIMPLET_OK) {
      simplet_list_iter_free(iter);
      set_error(map, layer->status, layer->error_msg);
      break;
    }
  }

  simplet_lithograph_free(litho);
  cairo_destroy(ctx);
  cairo_destroy(litho_ctx);
  return surface;
}

// Free the surface we've created.
static void
close_surface(cairo_surface_t *surface){
  cairo_surface_destroy(surface);
}

// Render the map and emit a stream of chunks to closure
void
simplet_map_render_to_stream(simplet_map_t *map, void *stream,
  cairo_status_t (*cb)(void *closure, const unsigned char *data, unsigned int length)){
  cairo_surface_t *surface;
  if(!(surface = simplet_map_build_surface(map))) return;

  if(cairo_surface_write_to_png_stream(surface, cb, stream) != CAIRO_STATUS_SUCCESS)
    set_error(map, SIMPLET_CAIRO_ERR, cairo_status_to_string(cairo_surface_status(surface)));

  close_surface(surface);
}

// Render the map to a file on disk.
void
simplet_map_render_to_png(simplet_map_t *map, const char *path){
  cairo_surface_t *surface;
  if(!(surface = simplet_map_build_surface(map))) return;

  if(cairo_surface_write_to_png(surface, path) != CAIRO_STATUS_SUCCESS)
    set_error(map, SIMPLET_CAIRO_ERR, cairo_status_to_string(cairo_surface_status(surface)));

  close_surface(surface);
}

