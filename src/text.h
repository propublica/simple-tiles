#ifndef _SIMPLE_TEXT_H
#define _SIMPLE_TEXT_H

#include "types.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  cairo_t *ctx;
  simplet_list_t *placements;
  simplet_list_t *styles;
} simplet_lithograph_t;


simplet_lithograph_t *
simplet_lithograph_new(cairo_t *ctx, simplet_list_t *styles);

void
simplet_lithograph_free(simplet_lithograph_t *litho);

void
simplet_lithograph_add_placement(simplet_lithograph_t *litho, OGRFeatureH feature, cairo_t *proj_ctx);

void
simplet_lithograph_apply(simplet_lithograph_t *litho);

#ifdef __cplusplus
}
#endif

#endif