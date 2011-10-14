#include "error.h"
#include <pthread.h>
#include <assert.h>

static pthread_mutex_t simplet_lock = PTHREAD_MUTEX_INITIALIZER;
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
  if(pthread_mutex_lock(&simplet_lock) > 0) return;
  OGRRegisterAll();
  simplet_error_init();
  atexit(cleanup);
  initialized = 1;
  pthread_mutex_unlock(&simplet_lock);
};
