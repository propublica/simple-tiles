#include "test.h"
#include <simple-tiles/rule.h>
#include <simple-tiles/style.h>

static void
test_rule(){
  simplet_rule_t *rule;
  const char *query = "SELECT * FROM TEST";
  if(!(rule = simplet_rule_new(query)))
    assert(0);
  assert(strcmp(query, rule->ogrsql) == 0);
  simplet_rule_add_style(rule, "fill", "#CCCCCC");
  simplet_style_t *style;
  style = rule->styles->tail->value;
  assert(strcmp(style->key, "fill") == 0);
  assert(strcmp(style->arg, "#CCCCCC") == 0);
}

static void
test_lookup(){
  simplet_rule_t *rule;
  if(!(rule = simplet_rule_new("SELECT * FROM TEST;")))
    assert(0);
  simplet_rule_add_style(rule, "fill",     "#CCCCCC");
  simplet_rule_add_style(rule, "stroke",   "#CCCCAA");
  simplet_rule_add_style(rule, "line-cap", "round");
  assert(rule->styles->length == 3);
}

TASK(rule) {
  test(rule);
  test(lookup);
}