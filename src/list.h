#ifndef _SIMPLE_TILES_LIST_H
#define _SIMPLE_TILES_LIST_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

simplet_list_t*
simplet_list_new();

void*
simplet_list_push(simplet_list_t* list, void* val);

void*
simplet_list_pop(simplet_list_t* list);

void
simplet_list_free(simplet_list_t* list);

void*
simplet_list_get(simplet_list_t* list, unsigned int idx);

simplet_listiter_t*
simplet_get_list_iter(simplet_list_t* list);

void
simplet_list_iter_free(simplet_listiter_t* iter);

void
simplet_list_set_item_free(simplet_list_t *list, simplet_list_item_free free);

void*
simplet_list_next(simplet_listiter_t* iter);



#ifdef __cplusplus
}
#endif

#endif
