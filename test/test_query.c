#include "test.h"
#include "query.h"
#include "style.h"

static void test_query() {
  simplet_query_t *query;
  const char *sql = "SELECT * FROM TEST";
  if (!(query = simplet_query_new(sql))) assert(0);

  assert(query->refcount == 1);
  assert(!strcmp(sql, query->ogrsql));

  simplet_query_add_style(query, "fill", "#CCCCCC");
  simplet_style_t *style;
  style = simplet_list_tail(query->styles);
  assert(!strcmp(style->key, "fill"));
  assert(!strcmp(style->arg, "#CCCCCC"));
  simplet_query_free(query);
}

static void test_lookup() {
  simplet_query_t *query;
  if (!(query = simplet_query_new("SELECT * FROM TEST;"))) assert(0);
  simplet_query_add_style(query, "fill", "#CCCCCC");
  simplet_query_add_style(query, "stroke", "#CCCCAA");
  simplet_query_add_style(query, "line-cap", "round");
  assert(simplet_list_get_length(query->styles) == 3);
  simplet_query_free(query);
}

static void test_getters() {
  const char *test = "SELECT * FROM TEST;";
  simplet_query_t *query;
  if (!(query = simplet_query_new(test))) assert(0);

  char *tmp;
  simplet_query_get(query, &tmp);
  assert(!strcmp(test, tmp));
  free(tmp);

  const char *t = "SELECT * FROM TEST2;";
  simplet_query_set(query, t);
  simplet_query_get(query, &tmp);
  assert(!strcmp(t, tmp));
  free(tmp);

  simplet_query_free(query);
}

static void test_user_data() {
  simplet_query_t *query;
  if (!(query = simplet_query_new("SELECT * FROM TEST;"))) assert(0);
  int i = 1;
  simplet_query_set_user_data(query, &i);
  int *j;
  j = simplet_query_get_user_data(query);
  assert(i == *j);
  simplet_query_free(query);
}

TASK(query) {
  test(query);
  test(lookup);
  test(getters);
  test(user_data);
}
