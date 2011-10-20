#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "util.h"

char*
simplet_copy_string(const char *src){
  if(src == NULL) return NULL;
  return strdup(src);
}

int
simplet_parse_color(const char *src, unsigned int *r, unsigned int *g,
                    unsigned int *b, unsigned int *a){
  return sscanf(src, "#%2x%2x%2x%2x", r, g, b, a);
}

