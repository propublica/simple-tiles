#ifndef _SIMPLE_TYPES_H
#define _SIMPLE_TYPES_H

#include <ogr_api.h>
#include <ogr_srs_api.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SIMPLET_ERROR_FIELDS \
    simplet_error_t error;

/* bounds and simple points */
typedef struct {
  double x;
  double y;
} simplet_point_t;

typedef struct {
  simplet_point_t nw;
  simplet_point_t se;
  double width;
  double height;
} simplet_bounds_t;


/* lists, nodes, and iterators */
typedef struct simplet_node_t {
  struct simplet_node_t *next;
  struct simplet_node_t *prev;
  void *value;
} simplet_node_t;


typedef void (*simplet_list_item_free)(void *val);
typedef struct simplet_list_t {
  simplet_node_t *head;
  simplet_node_t *tail;
  simplet_list_item_free free;
  unsigned int length;
} simplet_list_t;

typedef struct simplet_listiter_t {
  simplet_node_t *next;
} simplet_listiter_t;

/* errors */
typedef enum {
  SIMPLET_ERR = 0,
  SIMPLET_OOM,
  SIMPLET_CAIRO_ERR,
  SIMPLET_OGR_ERR,
  SIMPLET_OK
} simplet_status_t;

#define SIMPLET_MAX_ERROR 1024
typedef struct {
  simplet_status_t status;
  char msg[SIMPLET_MAX_ERROR];
} simplet_error_t;

/* map structures */
typedef struct {
  SIMPLET_ERROR_FIELDS
} simplet_errorable_t;


typedef struct {
  SIMPLET_ERROR_FIELDS
  simplet_bounds_t     *bounds;
  simplet_list_t       *layers;
  OGRSpatialReferenceH proj;
  double buffer; // pixel coords
  unsigned int width;
  unsigned int height;
  char *bgcolor;
} simplet_map_t;

typedef struct {
  SIMPLET_ERROR_FIELDS
  char           *source;
  simplet_list_t *filters;
} simplet_layer_t;

typedef struct {
  SIMPLET_ERROR_FIELDS
  char *ogrsql;
  simplet_list_t *styles;
} simplet_filter_t;

typedef struct {
  char *key;
  char *arg;
} simplet_style_t;


#ifndef M_PI
#define SIMPLET_PI acos(-1.0)
#else
#define SIMPLET_PI M_PI
#endif

#define SIMPLET_MERCATOR "epsg:3785"
#define SIMPLET_WGS84    "epsg:4326"


#ifdef __cplusplus
}
#endif

#endif
