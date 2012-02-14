#include "runner.h"

// run each test!
int
main(){
  task_wrap_t *task = (task_wrap_t*)&tasks;
  for(task = (task_wrap_t*)&tasks; task->call; task++){
    printf("%s\n", task->name);
    task->call();
  }
}
