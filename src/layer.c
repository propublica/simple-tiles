#include "layer.h"
#include "rule.h"

simplet_layer_t*
simplet_layer_new(const char *datastring){
  OGRRegisterAll();
  simplet_layer_t *layer;

  if(!(layer = malloc(sizeof(*layer))))
    return NULL;
    
  if(!(layer->source = OGROpen(datastring, 0, NULL))){
    free(layer);
    return NULL;
  }
  
  if(!(layer->rules = simplet_list_new())){
    OGR_DS_Destroy(layer->source);
    free(layer);
    return NULL;
  }
  
  return layer;
}

void
simplet_layer_vfree(void *layer){
  simplet_layer_free(layer);
}

void
simplet_layer_free(simplet_layer_t *layer){
  OGR_DS_Destroy(layer->source);
  layer->rules->free = simplet_rule_vfree;
  simplet_list_free(layer->rules);
  free(layer);
}

simplet_rule_t*
simplet_layer_add_rule(simplet_layer_t *layer, const char *ogrsql){
  simplet_rule_t* rule;
  if(!(rule = simplet_rule_new(ogrsql)))
    return NULL;
  
  if(!simplet_list_push(layer->rules, rule)){
    simplet_rule_free(rule);
    return NULL;
  }
  
  return rule;
}

int
simplet_layer_process(simplet_layer_t *layer, simplet_map_t *map){
  simplet_listiter_t *iter;
  if(!(iter = simplet_get_list_iter(layer->rules)))
    return 0;
  simplet_rule_t *rule;
  while((rule = simplet_list_next(iter)))
    simplet_rule_process(rule, layer, map);
  return 1;
}