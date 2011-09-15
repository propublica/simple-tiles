#include "error.h"
#include <pthread.h>

static pthread_mutex_t error_lock = PTHREAD_MUTEX_INITIALIZER;
static int initialized = 0;

// TODO: probably needs locking.
static void
cleanup(){
	for(int i = 0; i < OGRGetOpenDSCount(); i++)
		OGR_DS_Destroy(OGRGetOpenDS(i));
	OGRCleanupAll();
}

void
simplet_init(){
  if(initialized) return;
  if(pthread_mutex_lock(&error_lock) > 0) return;
  OGRRegisterAll();
  simplet_error_init();
  atexit(cleanup);
  initialized = 1;
  pthread_mutex_unlock(&error_lock);
};
