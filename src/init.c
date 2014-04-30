#include <pthread.h>
#include <assert.h>
#include <cpl_conv.h>
#include "error.h"

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
  CPLSetConfigOption("OGR_ENABLE_PARTIAL_REPROJECTION", "ON");
  #ifdef DEBUG
    CPLSetConfigOption("CPL_DEBUG", "ON");
  #endif
  OGRRegisterAll();
  GDALAllRegister();
  atexit(cleanup);
  initialized = 1;
};
