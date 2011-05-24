#include "test.h"
#include <simple-tiles/rule.h>
#include <simple-tiles/style.h>


static void
test_style(){
  simplet_style_t *style;
  if(!(style = simplet_style_new("fill", "#CCCCCC")))
    assert(0);
  assert(strcmp(style->key, "fill") == 0);
  assert(strcmp(style->arg, "#CCCCCC") == 0);
  simplet_style_free(style);
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
  simplet_style_t *style;
  style = simplet_lookup_style(rule->styles, "fill");
  assert(strcmp(style->key, "fill") == 0);
  assert(strcmp(style->arg, "#CCCCCC") == 0);
  style = simplet_lookup_style(rule->styles, "stroke");
  assert(strcmp(style->key, "stroke") == 0);
  assert(strcmp(style->arg, "#CCCCAA") == 0);
  style = simplet_lookup_style(rule->styles, "line-cap");
  assert(strcmp(style->key, "line-cap") == 0);
  assert(strcmp(style->arg, "round") == 0);
  simplet_rule_free(rule);
}

TASK(style) {
  test(style);
  test(lookup);
}