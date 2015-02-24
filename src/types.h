#ifndef _SIMPLE_TYPES_H
#define _SIMPLE_TYPES_H

#include <ogr_api.h>
#include <ogr_srs_api.h>
#include <gdal.h>
#include <cairo.h>
#include <pango/pangocairo.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif



typedef void (*simplet_user_data_free)(void *val);
#define SIMPLET_USER_DATA \
  void *user_data;
#define SIMPLET_FREEFUNC \
  simplet_user_data_free free;

/* lists, nodes, and iterators */
typedef struct simplet_node_t {
  struct simplet_node_t *next;
  struct simplet_node_t *prev;
  SIMPLET_USER_DATA
} simplet_node_t;

typedef struct simplet_list_t {
  simplet_node_t *head;
  simplet_node_t *tail;
  SIMPLET_FREEFUNC
  unsigned int length;
} simplet_list_t;

typedef struct simplet_listiter_t {
  simplet_node_t *next;
} simplet_listiter_t;

/* errors */
typedef enum {
  SIMPLET_ERR = 0,   // Generic error
  SIMPLET_OOM,       // Out of memory for allocation
  SIMPLET_CAIRO_ERR, // Cairo error
  SIMPLET_OGR_ERR,   // OGR error
  SIMPLET_GDAL_ERR,  // GDAL error
  SIMPLET_OK,        // OK, needs be here because of legacy reasons
  SIMPLET_GL_ERR     // OpenGL error
} simplet_status_t;

#define SIMPLET_ERROR_FIELDS \
  simplet_status_t status; \
  char *error_msg;

#define SIMPLET_RETAIN \
  int refcount;

/* map structures */
typedef struct {
  SIMPLET_ERROR_FIELDS
} simplet_errorable_t;

typedef struct {
  SIMPLET_ERROR_FIELDS
  SIMPLET_USER_DATA
} simplet_with_user_data_t;

typedef struct {
  SIMPLET_ERROR_FIELDS
  SIMPLET_USER_DATA
  SIMPLET_RETAIN
} simplet_retainable_t;

/* bounds and simple points */
typedef struct {
  double x;
  double y;
} simplet_point_t;

typedef struct {
  SIMPLET_ERROR_FIELDS
  SIMPLET_USER_DATA
  SIMPLET_RETAIN
  simplet_point_t nw;
  simplet_point_t se;
  double width;
  double height;
} simplet_bounds_t;

typedef struct {
  SIMPLET_ERROR_FIELDS
  SIMPLET_USER_DATA
  SIMPLET_RETAIN
  simplet_bounds_t  *bounds;
  simplet_list_t *layers;
  OGRSpatialReferenceH proj;
  double buffer; // pixel coords
  unsigned int width;
  unsigned int height;
  char *bgcolor;
} simplet_map_t;

typedef enum {
  SIMPLET_VECTOR,
  SIMPLET_RASTER
} simplet_layer_type_t;

#define SIMPLET_LAYER_FIELDS \
  SIMPLET_ERROR_FIELDS \
  SIMPLET_USER_DATA \
  SIMPLET_RETAIN \
  simplet_layer_type_t type; \
  char *source;

typedef struct {
  SIMPLET_LAYER_FIELDS
} simplet_layer_t;

typedef struct {
  SIMPLET_LAYER_FIELDS
  simplet_list_t *queries;
} simplet_vector_layer_t;

typedef struct {
  SIMPLET_LAYER_FIELDS
  bool resample;
} simplet_raster_layer_t;

typedef struct {
  SIMPLET_ERROR_FIELDS
  SIMPLET_USER_DATA
  SIMPLET_RETAIN
  char *ogrsql;
  simplet_list_t *styles;
} simplet_query_t;

typedef struct {
  SIMPLET_ERROR_FIELDS
  SIMPLET_USER_DATA
  SIMPLET_RETAIN
  char *key;
  char *arg;
} simplet_style_t;


#ifndef M_PI
#define SIMPLET_PI acos(-1.0)
#else
#define SIMPLET_PI M_PI
#endif

#define SIMPLET_MERCATOR "EPSG:3857"
#define SIMPLET_WGS84    "EPSG:4326"


#ifdef __cplusplus
}
#endif

#endif
