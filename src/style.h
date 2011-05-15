#ifndef _SIMPLE_TILES_STYLE_H
#define _SIMPLE_TILES_STYLE_H


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char *key;
  void (*call)(cairo_t *ctx, char *arg);
  char *arg;
} simplet_style_t;


void
simplet_apply_styles(cairo_t *ctx, simplet_list_t *styles);

void
simplet_style_fill(cairo_t *ctx, char *arg);

void
simplet_style_stroke(cairo_t *ctx, char *arg);

void
simplet_style_weight(cairo_t *ctx, char *arg);

simplet_style_t*
simplet_lookup_style(char *key);


#ifdef __cplusplus
}
#endif

#endif