#ifndef _SIMPLE_TILES_STYLE_H
#define _SIMPLE_TILES_STYLE_H

#include <cairo/cairo.h>
#include "types.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

void
simplet_style_fill(cairo_t *ctx, char *arg);

void
simplet_style_stroke(cairo_t *ctx, char *arg);

void
simplet_style_weight(cairo_t *ctx, char *arg);

void
simplet_style_line_join(cairo_t *ctx, char *arg);

void
simplet_style_line_cap(cairo_t *ctx, char *arg);

simplet_style_t*
simplet_style_new(char *key, char *arg);

void
simplet_style_vfree(void *style);

void
simplet_style_free(simplet_style_t* style);

void
simplet_apply_styles(cairo_t *ctx, simplet_list_t* styles, int count, ...);

simplet_style_t*
simplet_lookup_style(simplet_list_t* styles, char *key);

#ifdef __cplusplus
}
#endif

#endif