#include "error.h"
#include <pthread.h>
#include <assert.h>

static int initialized = 0;

// The atexit handler used to close all connections to open data stores
static void
cleanup(){
	for(int i = 0; i < OGRGetOpenDSCount(); i++)
    while(OGRGetOpenDS(i) && OGR_DS_GetRefCount(OGRGetOpenDS(i)))
      OGRReleaseDataSource(OGRGetOpenDS(i));
  assert(!OGRGetOpenDSCount());
  OGRCleanupAll();
}


// Initialize libraries, register the atexit handler and set up error reporting.
void
simplet_init(){
  if(initialized) return;
  simplet_error_init();
  OGRRegisterAll();
  atexit(cleanup);
  initialized = 1;
};
