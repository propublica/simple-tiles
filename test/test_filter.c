#include "test.h"
#include <simple-tiles/filter.h>
#include <simple-tiles/style.h>

static void
test_filter(){
  simplet_filter_t *filter;
  const char *query = "SELECT * FROM TEST";
  if(!(filter = simplet_filter_new(query)))
    assert(0);
  assert(!strcmp(query, filter->ogrsql));
  simplet_filter_add_style(filter, "fill", "#CCCCCC");
  simplet_style_t *style;
  style = filter->styles->tail->value;
  assert(!strcmp(style->key, "fill"));
  assert(!strcmp(style->arg, "#CCCCCC"));
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
}

TASK(filter) {
  test(filter);
  test(lookup);
}