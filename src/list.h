#ifndef __SIMPLE_TILES_LIST_H__
#define __SIMPLE_TILES_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct simplet_node_t {
  struct simplet_node_t *next;
  struct simplet_node_t *prev;
  void *value;
} simplet_node_t;

typedef struct simplet_list_t {
  simplet_node_t *head;
  simplet_node_t *tail;
  void (*free)(void *val);
  unsigned int length;
} simplet_list_t;

typedef struct simplet_listiter_t {
  simplet_node_t *next;
} simplet_listiter_t;

simplet_list_t*
simplet_list_new();

void*
simplet_list_push(simplet_list_t* list, void* val);

void*
simplet_list_pop(simplet_list_t* list);

void
simplet_list_free(simplet_list_t* list);

simplet_listiter_t*
simplet_get_list_iter(simplet_list_t* list);

void*
simplet_list_next(simplet_listiter_t* iter);

#ifdef __cplusplus
}
#endif

#endif