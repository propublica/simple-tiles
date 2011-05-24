#include "runner.h"

int
main(){
  task_wrap_t *task = (task_wrap_t*)&tasks;
  for(task = (task_wrap_t*)&tasks; task->call; task++){
    printf("Testing: %s\n", task->name);
    task->call();
  }
}