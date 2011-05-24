#ifndef _SIMPLE_TILES_RUNNER_H
#define _SIMPLE_TILES_RUNNER_H

#include "test.h"

typedef struct {
  void (*call)();
  const char *name;
} task_wrap_t;

task_wrap_t tasks[] = {
  TASK_ENTRY(list)
  TASK_ENTRY(layer)
  TASK_ENTRY(rule)
  TASK_ENTRY(style)
  { NULL, NULL }
};
#endif