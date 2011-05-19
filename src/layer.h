#ifndef _SIMPLET_LAYER_H
#define _SIMPLET_LAYER_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

simplet_layer_t*
simplet_layer_new(char *datastring);

void
simplet_layer_vfree(void *layer);

void
simplet_layer_free(simplet_layer_t *layer);

int
simplet_layer_process(simplet_layer_t *layer, simplet_map_t *map);

#ifdef __cplusplus
}
#endif

#endif