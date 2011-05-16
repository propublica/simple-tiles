#include <stdlib.h>
#include <string.h>

#include "util.h"

char*
simplet_copy_string(char *src){
  int len = strlen(src);
  char *dest;
  if(!(dest = malloc(len + 1)))
    return NULL;
  memcpy(dest, src, len);
  dest[len] = '\0';
  return dest;
}
