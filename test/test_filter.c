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
  style = simplet_list_tail(filter->styles);
  assert(!strcmp(style->key, "fill"));
  assert(!strcmp(style->arg, "#CCCCCC"));
  simplet_filter_free(filter);
}

static void
test_lookup(){
  simplet_filter_t *filter;
  if(!(filter = simplet_filter_new("SELECT * FROM TEST;")))
    assert(0);
  simplet_filter_add_style(filter, "fill",     "#CCCCCC");
  simplet_filter_add_style(filter, "stroke",   "#CCCCAA");
  simplet_filter_add_style(filter, "line-cap", "round");
  assert(simplet_list_get_length(filter->styles) == 3);
  simplet_filter_free(filter);
}

static void
test_query(){
  const char* test = "SELECT * FROM TEST;";
  simplet_filter_t *filter;
  if(!(filter = simplet_filter_new(test)))
    assert(0);

  char *tmp;
  simplet_filter_get_query(filter, &tmp);
  assert(!strcmp(test, tmp));
  free(tmp);

  const char* t = "SELECT * FROM TEST2;";
  simplet_filter_set_query(filter, t);
  simplet_filter_get_query(filter, &tmp);
  assert(!strcmp(t, tmp));
  free(tmp);

  simplet_filter_free(filter);
}

TASK(filter) {
  test(filter);
  test(lookup);
  test(query);
}
