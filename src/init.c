#include "error.h"
#include <pthread.h>
#include <assert.h>

static int initialized = 0;

static void
cleanup(){
	for(int i = 0; i < OGRGetOpenDSCount(); i++)
    while(OGR_DS_GetRefCount(OGRGetOpenDS(i))) //ughh
      OGRReleaseDataSource(OGRGetOpenDS(i));
  assert(!OGRGetOpenDSCount());
  OGRCleanupAll();
}

void
simplet_init(){
  if(initialized) return;
  OGRRegisterAll();
  simplet_error_init();
  atexit(cleanup);
  initialized = 1;
};
