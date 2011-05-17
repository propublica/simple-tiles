#include <stdlib.h>

#include "style.h"
#include "rule.h"
#include "util.h"


simplet_rule_t *
simplet_rule_new(char *sqlquery){
  simplet_rule_t *rule;
  if(!(rule = malloc(sizeof(*rule))))
    return NULL;

  if(!(rule->styles = simplet_list_new())){
    free(rule);
    return NULL;
  }
  
  rule->ogrsql = simplet_copy_string(sqlquery);
  
  return rule;
}

void
simplet_rule_free(simplet_rule_t *rule){
  simplet_list_t* styles = rule->styles;
  styles->free = simplet_style_vfree;
  simplet_list_free(styles);
  free(rule->ogrsql);
  free(rule);
}

void
simplet_rule_vfree(void *rule){
  simplet_rule_t *tmp = rule;
  simplet_rule_free(tmp);
}