#ifndef _SIMPLE_TILES_RULE_H
#define _SIMPLE_TILES_RULE_H

#include "list.h"
#include "map.h"

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

int
simplet_rule_process(simplet_map_t *map, simplet_rule_t *rule);

#ifdef __cplusplus
}
#endif

#endif
