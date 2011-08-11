#ifndef _SIMPLE_TILES_UTIL_H
#define _SIMPLE_TILES_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

char*
simplet_copy_string(const char *src);

int
simplet_parse_color(const char *src, unsigned int *r, unsigned int *g,
                    unsigned int *b, unsigned int *a);

#define SIMPLET_CCEIL 256.0
#define SIMPLET_CAIRO_RGBA(fn, ctx, arg)                                    \
  unsigned int r, g, b, a, count;                                           \
  count = simplet_parse_color(arg, &r, &g, &b, &a);                         \
  switch(count){                                                            \
  case 3:                                                                   \
    fn##_rgb(ctx, r / SIMPLET_CCEIL, g / SIMPLET_CCEIL, b / SIMPLET_CCEIL); \
    break;                                                                  \
  case 4:                                                                   \
    fn##_rgba(ctx, r / SIMPLET_CCEIL, g / SIMPLET_CCEIL, b / SIMPLET_CCEIL, \
        a / SIMPLET_CCEIL);                                                 \
    break;                                                                  \
  default:                                                                  \
    return;                                                                 \
  }                                                                         \


#ifdef __cplusplus
}
#endif

#endif
