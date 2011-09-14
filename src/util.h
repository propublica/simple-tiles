#ifndef _SIMPLE_TILES_UTIL_H
#define _SIMPLE_TILES_UTIL_H
#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif

char*
simplet_copy_string(const char *src);

int
simplet_parse_color(const char *src, unsigned int *r, unsigned int *g,
                    unsigned int *b, unsigned int *a);

#define SIMPLET_CCEIL 256.0

#ifdef __cplusplus
}
#endif

#endif
