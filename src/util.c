#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "util.h"

// A safe copy string.
char*
simplet_copy_string(const char *src){
  if(src == NULL) src = "";
  return strdup(src);
}

// Parse a color in hex form.
int
simplet_parse_color(const char *src, unsigned int *r, unsigned int *g,
                    unsigned int *b, unsigned int *a){
  return sscanf(src, "#%2x%2x%2x%2x", r, g, b, a);
}

