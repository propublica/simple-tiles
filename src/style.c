#include "map.h"
#include "style.h"
#include "util.h"

simplet_style_t styleTable[] = {
  { "fill",   simplet_style_fill,   NULL},
  { "stroke", simplet_style_stroke, NULL},
  { "weight", simplet_style_weight, NULL}
};

const int SIMPLET_STYLES_LENGTH = sizeof(simplet_style_t) / sizeof(*styleTable);

void
simplet_style_fill(cairo_t *ctx, char *arg){
  
}

void
simplet_style_stroke(cairo_t *ctx, char *arg){
  
}

void
simplet_style_weight(cairo_t *ctx, char *arg){
  
}

void
simplet_apply_styles(cairo_t *ctx, simplet_list_t *styles){
  simplet_listiter_t *iter = simplet_get_list_iter(styles);
  simplet_style_t *style;
  while((style = simplet_list_next(iter)))
    style->call(ctx, style->arg);
}


simplet_style_t*
simplet_lookup_style(char *key){
  simplet_style_t* style;
  if(!(style = malloc(sizeof(*style))))
    return NULL;
  
  for(int i = 0; i < SIMPLET_STYLES_LENGTH; i++){
    if(strcmp(key, styleTable[i].key)){
      style->call = styleTable[i].call;
      style->key  = copy_string(styleTable[i].key);
    }
  }
  
  return style;
}