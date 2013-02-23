#include "test.h"
#include "query.h"
#include "style.h"


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
  simplet_query_t *query;
  if(!(query = simplet_query_new("SELECT * FROM TEST;")))
    assert(0);
  simplet_query_add_style(query, "fill",     "#CCCCCC");
  simplet_query_add_style(query, "stroke",   "#CCCCAA");
  simplet_query_add_style(query, "line-cap", "round");

  assert(query->styles->length == 3);
  simplet_style_t *style;
  style = simplet_lookup_style(query->styles, "fill");
  assert(!strcmp(style->key, "fill"));
  assert(!strcmp(style->arg, "#CCCCCC"));
  style = simplet_lookup_style(query->styles, "stroke");
  assert(!strcmp(style->key, "stroke"));
  assert(!strcmp(style->arg, "#CCCCAA"));
  style = simplet_lookup_style(query->styles, "line-cap");
  assert(!strcmp(style->key, "line-cap"));
  assert(!strcmp(style->arg, "round"));
  simplet_query_free(query);
}

TASK(style) {
  test(style);
  test(lookup);
}
