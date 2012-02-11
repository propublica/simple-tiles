#include <stdarg.h>
#include <stdlib.h>

#include "map.h"
#include "style.h"
#include "util.h"

// Small structure to track callbacks by key.
typedef struct simplet_styledef_t {
  const char *key;
  void (*call)(void *ct, const char *arg);
} simplet_styledef_t;



// Set up user data functions on simplet_style_t.
SIMPLET_HAS_USER_DATA(style)

// Set the current drawing color for the ctx. Accepts either
// #xxxxxx or #xxxxxxaa formatted colors.
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

// Set the line join on the ct.
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

// Set the ending line cap on the ct.
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

// Paint an overlay color on the ct.
void
simplet_style_paint(void *ct, const char *arg){
  cairo_t *ctx = ct;
  set_color(ctx, arg);
  cairo_paint(ctx);
}

// Fill the current path in ctx.
static void
fill(void *ct, const char *arg){
  cairo_t *ctx = ct;
  set_color(ctx, arg);
  cairo_fill_preserve(ctx);
}

// Draw the current path in ctx with color arg.
static void
stroke(void *ct, const char *arg){
  cairo_t *ctx = ct;
  set_color(ctx, arg);
  cairo_stroke_preserve(ctx);
}

// Set the line weight on the ctx.
static void
weight(void *ct, const char *arg){
  cairo_t *ctx = ct;
  double w = strtod(arg, NULL), y = 0;
  cairo_device_to_user_distance(ctx, &w, &y);
  cairo_set_line_width(ctx, w);
}

// Set the letter spacing on for typsetting on a PangoLayout.
static void
letter_spacing(void *ct, const char *arg){
  PangoAttribute *spacing;
  if(!(spacing = pango_attr_letter_spacing_new(atoi(arg) * PANGO_SCALE))) return;

  PangoLayout *layout = ct;
  PangoAttrList *attrs = pango_layout_get_attributes(layout);

  // Create a new PangoAttrList if we don't already have one.
  if(!attrs) {
    if(!(attrs = pango_attr_list_new())) return;
  } else {
    pango_attr_list_ref(attrs);
  }

  // Insert the spacing definition.
  pango_attr_list_insert(attrs, spacing);
  pango_layout_set_attributes(layout, attrs);
  pango_attr_list_unref(attrs);
}


// List of defined styles.
simplet_styledef_t styleTable[] = {
  { "fill",                fill                    },
  { "stroke",              stroke                  },
  { "weight",              weight                  },
  { "line-cap",            line_cap                },
  { "color",               fill                    },
  { "text-stroke-color",   stroke                  },
  { "text-stroke-width",   weight                  },
  { "letter-spacing",      letter_spacing          },
  { "paint",               simplet_style_paint     }, //used by map
  { "line-join",           simplet_style_line_join }  //used by map
  /* radius and seamless are special styles */
};
const int STYLES_LENGTH = sizeof(styleTable) / sizeof(*styleTable);

// Create and return a new simplt_style_t or NULL on failure
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

// Free a simplet_style_t from a void pointer
void
simplet_style_vfree(void *style){
  simplet_style_free(style);
}

// Free a simplet_style_t
void
simplet_style_free(simplet_style_t* style){
  free(style->key);
  free(style->arg);
  free(style);
}

// Find a styledef in the styleTable.
static simplet_styledef_t*
lookup_styledef(char *key){
  for(int i = 0; i < STYLES_LENGTH; i++)
    if(!strcmp(key, styleTable[i].key))
      return &styleTable[i];
  return NULL;
}

// Apply styles to a ctx using variable args, the last argument must be a
// sentinel NULL.
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

// Find and return a style by key from a list of styles. This isn't a hash table,
// because you won't have a whole ton of styles O(N) is okey-dokey.
simplet_style_t*
simplet_lookup_style(simplet_list_t *styles, const char *key){
  simplet_listiter_t* iter;
  if(!(iter = simplet_get_list_iter(styles)))
    return NULL;

  simplet_style_t* style;
  while((style = simplet_list_next(iter))){
    // If we find the style that matches key.
    if(!strcmp(key, style->key)) {
      // Free the iterator and return the style.
      simplet_list_iter_free(iter);
      return style;
    }
  }
  return NULL;
}

// Return the arg for the style and store it in arg.
void
simplet_style_get_arg(simplet_style_t *style, char **arg){
  *arg = simplet_copy_string(style->arg);
}

// Get the style's key and store it in key.
void
simplet_style_get_key(simplet_style_t *style, char **key){
  *key = simplet_copy_string(style->key);
}

// Set a copy of arg in style.
void
simplet_style_set_arg(simplet_style_t *style, char *arg){
  style->arg = simplet_copy_string(arg);
}

// Set a copy of key in style.
void
simplet_style_set_key(simplet_style_t *style, char *key){
  style->key = simplet_copy_string(key);
}
