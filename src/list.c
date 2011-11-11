#include "list.h"
#include <stdlib.h>

simplet_list_t*
simplet_list_new(){
  simplet_list_t* list;
  if(!(list = malloc(sizeof(*list))))
    return NULL;
  
  memset(list, 0, sizeof(*list));

  return list;
}

void*
simplet_list_push(simplet_list_t *list, void* val){
  simplet_node_t *node;
  if(!(node = malloc(sizeof(*node))))
    return NULL;

  node->value = val;

  if(!(list->head || list->tail)) {
    list->head = list->tail = node;
    node->next = node->prev = NULL;
  } else {
    node->next = NULL;
    node->prev = list->tail;
    list->tail->next = node;
    list->tail = node;
  }

  list->length++;
  return val;
}

int
simplet_list_get_length(simplet_list_t *list){
  return list->length;
}

void*
simplet_list_tail(simplet_list_t *list){
  return list->tail->value;
}

void*
simplet_list_head(simplet_list_t *list){
  return list->head->value;
}

void*
simplet_list_pop(simplet_list_t *list){
  if(!list->tail)
    return NULL;

  simplet_node_t *node = list->tail;
  void *val = node->value;

  if (node->prev)
    node->prev->next = NULL;
  else
    list->head = NULL;

  list->tail = node->prev;

  free(node);
  list->length--;
  return val;
}

void*
simplet_list_get(simplet_list_t* list, unsigned int idx){
  if(idx > list->length) return NULL;
  simplet_node_t *n = list->head;
  if(idx <= 0) return n->value;
  while(idx-- && n) n = n->next;
  if(n) return n->value;
  return NULL;
}

void
simplet_list_free(simplet_list_t *list){
  void* val;
  while((val = simplet_list_pop(list)) != NULL)
    if(list->free) list->free(val);
  free(list);
}

void
simplet_list_set_item_free(simplet_list_t *list, simplet_list_item_free destroy){
  list->free = destroy;
}

void
simplet_list_iter_free(simplet_listiter_t* iter){
  free(iter);
}

simplet_listiter_t*
simplet_get_list_iter(simplet_list_t *list){
  simplet_listiter_t* iter;
  if(!(iter = malloc(sizeof(*iter))))
    return NULL;
  iter->next = list->head;
  return iter;
}

void*
simplet_list_next(simplet_listiter_t* iter){
  if(!iter) return NULL;
  simplet_node_t *current = iter->next;
  if(current != NULL) {
    iter->next = current->next;
    return current->value;
  } else {
    simplet_list_iter_free(iter);
  }
  return NULL;
}

