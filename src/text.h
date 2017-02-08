#ifndef _SIMPLE_TEXT_H
#define _SIMPLE_TEXT_H

#include "types.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  SIMPLET_ERROR_FIELDS
  SIMPLET_USER_DATA
  SIMPLET_RETAIN
  cairo_t *ctx;
  PangoContext *pango_ctx;
  simplet_list_t *placements;
} simplet_lithograph_t;

simplet_lithograph_t *simplet_lithograph_new(cairo_t *ctx);

void simplet_lithograph_free(simplet_lithograph_t *litho);

void simplet_lithograph_add_placement(simplet_lithograph_t *litho,
                                      OGRFeatureH feature,
                                      simplet_list_t *styles,
                                      cairo_t *proj_ctx);

void simplet_lithograph_apply(simplet_lithograph_t *litho,
                              simplet_list_t *styles);

#ifdef __cplusplus
}
#endif

#endif
