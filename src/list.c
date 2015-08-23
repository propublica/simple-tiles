#include "list.h"
#include <stdlib.h>

// Create a new list, returns NULL on failure.
simplet_list_t* simplet_list_new() {
  simplet_list_t* list;
  if (!(list = malloc(sizeof(*list)))) return NULL;

  memset(list, 0, sizeof(*list));

  return list;
}

// Push a void pointer on the the end of the list.
void* simplet_list_push(simplet_list_t* list, void* val) {
  simplet_node_t* node;
  if (!(node = malloc(sizeof(*node)))) return NULL;

  node->user_data = val;

  if (!(list->head || list->tail)) {
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

// Get the length of the list.
int simplet_list_get_length(simplet_list_t* list) { return list->length; }

// Get the last element on the list.
void* simplet_list_tail(simplet_list_t* list) {
  if (!list->tail) return NULL;
  return list->tail->user_data;
}

// Get the first element of the list.
void* simplet_list_head(simplet_list_t* list) {
  if (!list->head) return NULL;
  return list->head->user_data;
}

// Remove and return the last element of the list.
void* simplet_list_pop(simplet_list_t* list) {
  if (!list->tail) return NULL;

  simplet_node_t* node = list->tail;
  void* val = node->user_data;

  if (node->prev)
    node->prev->next = NULL;
  else
    list->head = NULL;

  list->tail = node->prev;

  free(node);
  list->length--;

  return val;
}

// Get an element at idx.
void* simplet_list_get(simplet_list_t* list, unsigned int idx) {
  if (idx > list->length) return NULL;
  simplet_node_t* n = list->head;
  if (idx <= 0) return n->user_data;
  while (idx-- && n) n = n->next;
  if (n) return n->user_data;
  return NULL;
}

// Free a list and call the previously set free function for eaxh element in the
// list.
void simplet_list_free(simplet_list_t* list) {
  void* val;
  while ((val = simplet_list_pop(list)) != NULL)
    if (list->free) list->free(val);
  free(list);
}

// Set the free function for a list.
void simplet_list_set_item_free(simplet_list_t* list,
                                simplet_user_data_free destroy) {
  list->free = destroy;
}

// Free a list iterator.
void simplet_list_iter_free(simplet_listiter_t* iter) { free(iter); }

// Return a list iterator. TODO: this doesn't really need to be on the heap,
// we should switch to stack allocation perhaps?
simplet_listiter_t* simplet_get_list_iter(simplet_list_t* list) {
  simplet_listiter_t* iter;
  if (!(iter = malloc(sizeof(*iter)))) return NULL;
  iter->next = list->head;
  return iter;
}

// Return the next value of the list via the list iterator.
void* simplet_list_next(simplet_listiter_t* iter) {
  if (!iter) return NULL;
  simplet_node_t* current = iter->next;
  if (current != NULL) {
    iter->next = current->next;
    return current->user_data;
  } else {
    simplet_list_iter_free(iter);
  }
  return NULL;
}
