#ifndef _SIMPLE_TILES_RULE_H
#define _SIMPLE_TILES_RULE_H

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char *ogrsql;
  simplet_list_t *styles;
} simplet_rule_t;

void
simplet_rule_vfree(void *rule);

void
simplet_rule_free(simplet_rule_t *rule);

simplet_rule_t *
simplet_rule_new(char *sqlquery);

#ifdef __cplusplus
}
#endif

#endif
