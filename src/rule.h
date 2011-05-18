#ifndef _SIMPLE_TILES_RULE_H
#define _SIMPLE_TILES_RULE_H

#include "types.h"
#include "list.h"
#include "map.h"
#include "style.h"

#ifdef __cplusplus
extern "C" {
#endif

void
simplet_rule_vfree(void *rule);

void
simplet_rule_free(simplet_rule_t *rule);

simplet_rule_t*
simplet_rule_new(char *sqlquery);

simplet_style_t*
simplet_rule_add_style(simplet_rule_t *rule, char *key, char *arg);

int
simplet_rule_process(simplet_map_t *map, simplet_rule_t *rule);

#ifdef __cplusplus
}
#endif

#endif
