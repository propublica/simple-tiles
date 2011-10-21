#include "test.h"
#include <simple-tiles/filter.h>
#include <simple-tiles/style.h>


static void
test_style(){
  simplet_style_t *style;
  if(!(style = simplet_style_new("fill", "#CCCCCC")))
    assert(0);
  assert(!strcmp(style->key, "fill"));
  assert(!strcmp(style->arg, "#CCCCCC"));
  simplet_style_free(style);
}

static void
test_lookup(){
  simplet_filter_t *filter;
  if(!(filter = simplet_filter_new("SELECT * FROM TEST;")))
    assert(0);
  simplet_filter_add_style(filter, "fill",     "#CCCCCC");
  simplet_filter_add_style(filter, "stroke",   "#CCCCAA");
  simplet_filter_add_style(filter, "line-cap", "round");

  assert(filter->styles->length == 3);
  simplet_style_t *style;
  style = simplet_lookup_style(filter->styles, "fill");
  assert(!strcmp(style->key, "fill"));
  assert(!strcmp(style->arg, "#CCCCCC"));
  style = simplet_lookup_style(filter->styles, "stroke");
  assert(!strcmp(style->key, "stroke"));
  assert(!strcmp(style->arg, "#CCCCAA"));
  style = simplet_lookup_style(filter->styles, "line-cap");
  assert(!strcmp(style->key, "line-cap"));
  assert(!strcmp(style->arg, "round"));
  simplet_filter_free(filter);
}

TASK(style) {
  test(style);
  test(lookup);
}
