#ifndef _SIMPLE_TILES_RUNNER_H
#define _SIMPLE_TILES_RUNNER_H

#include "test.h"

// an entry in our task runner
#define TASK_ENTRY(name) \
  { &run_task_##name, #name },

typedef struct {
  void (*call)();
  const char *name;
} task_wrap_t;

task_wrap_t tasks[] = {
  TASK_ENTRY(list)
  TASK_ENTRY(bounds)
  TASK_ENTRY(vector_layer)
  TASK_ENTRY(raster_layer)
  TASK_ENTRY(query)
  TASK_ENTRY(style)
  TASK_ENTRY(map)
  TASK_ENTRY(integration)
  { NULL, NULL }
};

#endif
