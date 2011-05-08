#include "list.h"
#include <stdlib.h>

simplet_list_t*
simplet_list_new(){
  simplet_list_t* list;
  if(!(list = malloc(sizeof(*list))))
    return NULL;
  list->head   = NULL;
  list->tail   = NULL;
  list->free   = NULL;
  list->length = 0;
  
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

// TODO: Fix this so as not to rely on length
void*
simplet_list_pop(simplet_list_t *list){
  if(list->length <= 0) return NULL;

  simplet_node_t *node = list->tail;
  void *val = node->value;
  
  if(--list->length) {
    list->tail = node->prev;
    list->tail->next = node->next;
  } else {
    list->tail = list->head = NULL;
  }

  //list->length--;
  free(node);
  return val;
}

void
simplet_list_free(simplet_list_t *list){
  void* val;
  while((val = simplet_list_pop(list)) != NULL)
    if(list->free) list->free(val);
  free(list);
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
simplet_get_next(simplet_listiter_t* iter){
  simplet_node_t *current = iter->next;
  if(current != NULL) {
    iter->next = current->next;
    return current->value;
  } else {
    free(iter);
  }
  return NULL;
}



