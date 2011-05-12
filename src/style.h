#ifndef SIMPLE_TILES_STYLE_H
#define SIMPLE_TILES_STYLE_H


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char *key;
  void (*call)(cairo_t *ctx, char *arg);
  char *arg;
} simplet_style_t;

void
simplet_style_fill(cairo_t *ctx, char *arg);

void
simplet_style_stroke(cairo_t *ctx, char *arg);

void
simplet_style_weight(cairo_t *ctx, char *arg);

simplet_style_t styleTable[] = {
  { "fill",   simplet_style_fill,   NULL},
  { "stroke", simplet_style_stroke, NULL},
  { "weight", simplet_style_weight, NULL}
};

const int SIMPLET_STYLES_LENGTH = sizeof(simplet_style_t) / sizeof(*styleTable);


#ifdef __cplusplus
}
#endif

#endif