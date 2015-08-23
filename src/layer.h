#ifndef _SIMPLET_LAYER_H
#define _SIMPLET_LAYER_H

#include "types.h"
#include "text.h"
#include "user_data.h"

#ifdef __cplusplus
extern "C" {
#endif

void simplet_layer_vfree(void *layer);

void simplet_layer_get_source(simplet_layer_t *layer, char **source);

void simplet_layer_set_source(simplet_layer_t *layer, char *source);

#ifdef __cplusplus
}
#endif

#endif
