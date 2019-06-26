#include <pthread.h>
#include <assert.h>
#include <cpl_conv.h>
#include "error.h"

static int initialized = 0;

// Initialize libraries, register the atexit handler and set up error reporting.
void simplet_init() {
  if (initialized) return;
  CPLSetConfigOption("OGR_ENABLE_PARTIAL_REPROJECTION", "ON");
#ifdef DEBUG
  CPLSetConfigOption("CPL_DEBUG", "ON");
#else
  CPLSetConfigOption("CPL_DEBUG", "OFF");
#endif
  OGRRegisterAll();
  GDALAllRegister();
  initialized = 1;
};
