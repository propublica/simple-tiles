#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

#include "map.h"
#include "style.h"
#include "util.h"

// Refactor candidate.
// OGR has styles! http://www.gdal.org/ogr/ogr_feature_style.html
typedef struct simplet_styledef_t {
  char *key;
  void (*call)(cairo_t *ctx, char *arg);
} simplet_styledef_t;

simplet_styledef_t styleTable[] = {
  { "fill",   simplet_style_fill   },
  { "stroke", simplet_style_stroke },
  { "weight", simplet_style_weight }
};

const int STYLES_LENGTH = sizeof(styleTable) / sizeof(*styleTable);


static int
scan_color(char *str, unsigned int *r, unsigned int *g, unsigned int *b, unsigned int *a){
  return sscanf(str, "#%2x%2x%2x%2x", r, g, b, a);
}

void
simplet_style_fill(cairo_t *ctx, char *arg){
  unsigned int r, g, b, a, count;
  count = scan_color(arg, &r, &g, &b, &a);
  switch(count){
  case 3:
    cairo_set_source_rgb(ctx, r / 255.0, g / 255.0, b / 255.0);
    break;
  case 4:
    cairo_set_source_rgba(ctx, r / 255.0, g / 255.0, b / 255.0, a / 255.0);
    break;
  default:
    return;
  }
  cairo_fill_preserve(ctx);
}

void
simplet_style_stroke(cairo_t *ctx, char *arg){
  unsigned int r, g, b, a, count;
  count = scan_color(arg, &r, &g, &b, &a);
  switch(count){
  case 3:
    cairo_set_source_rgb(ctx, r / 255.0, g / 255.0, b / 255.0);
    break;
  case 4:
    cairo_set_source_rgba(ctx, r / 255.0, g / 255.0, b / 255.0, a / 255.0);
    break;
  default:
    return;
  }
  cairo_stroke_preserve(ctx);
}

void
simplet_style_weight(cairo_t *ctx, char *arg){
  cairo_set_line_width(ctx, strtod(arg, NULL));
}


simplet_style_t*
simplet_style_new(char *key, char *arg){
  simplet_style_t* style;
  if(!(style = malloc(sizeof(*style))))
    return NULL;
  
  style->key = simplet_copy_string(key);
  style->arg = simplet_copy_string(arg);
  
  return style;
}

void
simplet_style_vfree(void *style){
  simplet_style_t *tmp = style;
  simplet_style_free(tmp);
}

void
simplet_style_free(simplet_style_t* style){
  free(style->key);
  free(style->arg);
  free(style);
}

static simplet_styledef_t*
lookup_styledef(char *key){
  for(int i = 0; i < STYLES_LENGTH; i++)
    if(strcmp(key, styleTable[i].key) == 0)
      return &styleTable[i];
  return NULL;
}

void
simplet_apply_styles(cairo_t *ctx, simplet_list_t* styles, int count, ...){
  va_list args;
  va_start(args, count);
  char* key;
  for(int i = 0; i < count; i++){
    key = va_arg(args, char*);
    simplet_styledef_t* def = lookup_styledef(key);
    if(def == NULL)
      continue;
    
    simplet_style_t* style = simplet_lookup_style(styles, key);
    if(style == NULL)
      continue;
      
    def->call(ctx, style->arg);
  }
  va_end(args);
}

simplet_style_t*
simplet_lookup_style(simplet_list_t* styles, char *key){
  simplet_listiter_t* iter;
  if(!(iter = simplet_get_list_iter(styles)))
    return NULL;

  simplet_style_t* style;
  while((style = simplet_list_next(iter))){
    if(strcmp(key, style->key) == 0) {
      simplet_list_iter_free(iter);
      return style;
    }
  }
  
  return NULL;
}