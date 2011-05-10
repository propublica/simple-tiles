#ifndef __SIMPLE_TILES_H__
#define __SIMPLE_TILES_H__

#include "bounds.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SIMPLE_TILES_VERSION "0.0.1"

#include <stdio.h>

typedef struct {
  simplet_flist_t *styles; /* list of styles */
} simplet_styles_t;

typedef struct {
  simplet_flist_t *rules; /* list of rules */
} simplet_rules_t;

typedef struct {
  void (*declare)(cairo_t *ctx);
} simplet_declaration_t;

typedef struct {
  int (*filter)(OGRFeature *feature, char* key, char* value);
} simplet_filter_t;

typedef struct {
  OGRDataSourceH source;
  simplet_bounds_t *size;
  simplet_styles_t *styles;
  simplet_rules_t  *rules;
  projPJ  *proj;
  cairo_t *ctx;
} simplet_map_t;

typedef struct {
  simplet_t *nw;
  simplet_t *se;
  double width;
  double height;
} simplet_bounds_t;

enum simplet_filters {
  SIMPLET_FIELD_FILTER_LT,
  SIMPLET_FIELD_FILTER_LTE,
  SIMPLET_FIELD_FILTER_EQ,
  SIMPLET_FIELD_FILTER_GT,
  SIMPLET_FIELD_FILTER_GTE,
  SIMPLET_ALL
}

enum simplet_styles {
  SIMPLET_FILL,
  SIMPLET_STROKE
}

#ifdef __cplusplus
}
#endif

#endif
