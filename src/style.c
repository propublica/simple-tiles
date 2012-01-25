#include <stdarg.h>
#include <stdlib.h>

#include "map.h"
#include "style.h"
#include "util.h"

typedef struct simplet_styledef_t {
  const char *key;
  void (*call)(void *ct, const char *arg);
} simplet_styledef_t;

SIMPLET_HAS_USER_DATA(style)


static void
set_color(void *ct, const char *arg){
  cairo_t *ctx = ct;
  unsigned int r, g, b, a, count;
  count = simplet_parse_color(arg, &r, &g, &b, &a);
  switch(count){
  case 3:
    cairo_set_source_rgb(ctx, r / SIMPLET_CCEIL, g / SIMPLET_CCEIL, b / SIMPLET_CCEIL);
    break;
  case 4:
    cairo_set_source_rgba(ctx, r / SIMPLET_CCEIL, g / SIMPLET_CCEIL, b / SIMPLET_CCEIL,
        a / SIMPLET_CCEIL);
    break;
  default:
    return;
  }
}

void
simplet_style_line_join(void *ct, const char *arg){
  cairo_t *ctx = ct;
  if(!strcmp("miter", arg))
    cairo_set_line_join(ctx, CAIRO_LINE_JOIN_MITER);
  if(!strcmp("round", arg))
    cairo_set_line_join(ctx, CAIRO_LINE_JOIN_ROUND);
  if(!strcmp("bevel", arg))
    cairo_set_line_join(ctx, CAIRO_LINE_JOIN_BEVEL);
}

static void
line_cap(void *ct, const char *arg){
  cairo_t *ctx = ct;
  if(!strcmp("butt", arg))
    cairo_set_line_cap(ctx, CAIRO_LINE_CAP_BUTT);
  if(!strcmp("round", arg))
    cairo_set_line_cap(ctx, CAIRO_LINE_CAP_ROUND);
  if(!strcmp("square", arg))
    cairo_set_line_cap(ctx, CAIRO_LINE_CAP_SQUARE);
}

void
simplet_style_paint(void *ct, const char *arg){
  cairo_t *ctx = ct;
  set_color(ctx, arg);
  cairo_paint(ctx);
}

static void
fill(void *ct, const char *arg){
  cairo_t *ctx = ct;
  set_color(ctx, arg);
  cairo_fill_preserve(ctx);
}

static void
stroke(void *ct, const char *arg){
  cairo_t *ctx = ct;
  set_color(ctx, arg);
  cairo_stroke_preserve(ctx);
}

static void
weight(void *ct, const char *arg){
  cairo_t *ctx = ct;
  double w = strtod(arg, NULL), y = 0;
  cairo_device_to_user_distance(ctx, &w, &y);
  cairo_set_line_width(ctx, w);
}

static void
letter_spacing(void *ct, const char *arg){
  PangoAttribute *spacing;
  if(!(spacing = pango_attr_letter_spacing_new(atoi(arg) * PANGO_SCALE))) return;

  PangoLayout *layout = ct;
  PangoAttrList *attrs = pango_layout_get_attributes(layout);

  if(!attrs) {
    if(!(attrs = pango_attr_list_new())) return;
  } else {
    pango_attr_list_ref(attrs);
  }

  pango_attr_list_insert(attrs, spacing);
  pango_layout_set_attributes(layout, attrs);
  pango_attr_list_unref(attrs);
}

simplet_styledef_t styleTable[] = {
  { "fill",                fill                    },
  { "stroke",              stroke                  },
  { "weight",              weight                  },
  { "line-cap",            line_cap                },
  { "color",               fill                    },
  { "text-outline-color",  stroke                  },
  { "text-outline-weight", weight                  },
  { "letter-spacing",      letter_spacing          },
  { "paint",               simplet_style_paint     }, //used by map
  { "line-join",           simplet_style_line_join }  //used by map
  /* radius and seamless are special styles */
};

const int STYLES_LENGTH = sizeof(styleTable) / sizeof(*styleTable);

simplet_style_t*
simplet_style_new(const char *key, const char *arg){
  simplet_style_t* style;
  if(!(style = malloc(sizeof(*style))))
    return NULL;

  style->key = simplet_copy_string(key);
  style->arg = simplet_copy_string(arg);

  if(!(style->key && style->arg)){
    free(style);
    return NULL;
  }

  return style;
}

void
simplet_style_vfree(void *style){
  simplet_style_free(style);
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
    if(!strcmp(key, styleTable[i].key))
      return &styleTable[i];
  return NULL;
}

void
simplet_apply_styles(void *ct, simplet_list_t* styles, ...){
  va_list args;
  va_start(args, styles);
  char* key;
  while((key = va_arg(args, char*)) != NULL) {
    simplet_styledef_t* def = lookup_styledef(key);
    if(def == NULL)
      continue;

    simplet_style_t* style = simplet_lookup_style(styles, key);
    if(style == NULL)
      continue;

    def->call(ct, style->arg);
  }
  va_end(args);
}

simplet_style_t*
simplet_lookup_style(simplet_list_t *styles, const char *key){
  simplet_listiter_t* iter;
  if(!(iter = simplet_get_list_iter(styles)))
    return NULL;

  simplet_style_t* style;
  while((style = simplet_list_next(iter))){
    if(!strcmp(key, style->key)) {
      simplet_list_iter_free(iter);
      return style;
    }
  }
  return NULL;
}

// TODO: add oom to these which requires that styles be errorable.
void
simplet_style_get_arg(simplet_style_t *style, char **arg){
  *arg = simplet_copy_string(style->arg);
}

void
simplet_style_get_key(simplet_style_t *style, char **key){
  *key = simplet_copy_string(style->key);
}

void
simplet_style_set_arg(simplet_style_t *style, char *arg){
  style->arg = simplet_copy_string(arg);
}

void
simplet_style_set_key(simplet_style_t *style, char *key){
  style->key = simplet_copy_string(key);
}
