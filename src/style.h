#ifndef _SIMPLE_TILES_STYLE_H
#define _SIMPLE_TILES_STYLE_H

#include <cairo/cairo.h>
#include "types.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

void
simplet_style_line_join(void *ct, const char *arg);

void
simplet_style_paint(void *ct, const char *arg);

simplet_style_t*
simplet_style_new(const char *key, const char *arg);

void
simplet_style_vfree(void *style);

void
simplet_style_free(simplet_style_t* style);

void
simplet_apply_styles(void *ct, simplet_list_t* styles, ...);

simplet_style_t*
simplet_lookup_style(simplet_list_t* styles, const char *key);

void
simplet_style_get_arg(simplet_style_t* style, char **arg);

void
simplet_style_get_key(simplet_style_t* style, char **key);

void
simplet_style_set_arg(simplet_style_t *style, char *arg);

void
simplet_style_set_key(simplet_style_t *style, char *key);

#ifdef __cplusplus
}
#endif

#endif
