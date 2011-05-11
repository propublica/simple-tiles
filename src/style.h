#ifndef __SIMPLE_TILES_STYLE_H__
#define __SIMPLE_TILES_STYLE_H__


#ifdef __cplusplus
extern "C" {
#endif

struct simplet_style {
  char *name;
  void (*style)(cairo_t *ctx, char *arg);
};

void
simplet_style_fill(cairo_t *ctx, char *arg);

void
simplet_style_stroke(cairo_t *ctx, char *arg);

void
simplet_style_weight(cairo_t *ctx, char *arg);

struct simplet_style styleTable[] = {
  /* key,   callback           */
  { "fill",   simplet_style_fill   },
  { "stroke", simplet_style_stroke },
  { "weight", simplet_style_weight }
};

#ifdef __cplusplus
}
#endif

#endif