#ifndef _SIMPLE_TILES_TEST_H
#define _SIMPLE_TILES_TEST_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <simple-tiles/simple_tiles.h>

#define test(fn) \
        printf("\x1b[33m" # fn "\x1b[0m "); \
        test_##fn(); \
        puts("\x1b[1;32m✓ \x1b[0m");
        
#define TASK(name) \
  void run_task_##name()

TASK(list);
TASK(layer);
TASK(rule);
TASK(style);

#endif
