#include <cpl_error.h>
#include <pthread.h>
#include "error.h"


static pthread_mutex_t error_lock = PTHREAD_MUTEX_INITIALIZER;
static int error_initialized = 0;

static void
ogr_error_handler(CPLErr eclass, int err_no, const char *msg){
  (void)eclass, (void)err_no, (void)msg; // silence ogr errors
}

void
simplet_error_init(){
  if(error_initialized) return;
  if(pthread_mutex_lock(&error_lock) > 0) return;
  error_initialized = 1;
  CPLSetErrorHandler(ogr_error_handler);
  pthread_mutex_unlock(&error_lock);
}

void
simplet_set_error(simplet_error_t *error, simplet_status_t status, const char *msg){
  switch(status){
    case SIMPLET_ERR:
      error->status = SIMPLET_ERR;
      snprintf(error->msg, SIMPLET_MAX_ERROR, "simple tiles error: %s", msg);
      break;
    case SIMPLET_OOM:
      error->status = SIMPLET_OOM;
      snprintf(error->msg, SIMPLET_MAX_ERROR, "out of memory for allocation, %s", msg);
      break;
    case SIMPLET_CAIRO_ERR:
      error->status = SIMPLET_CAIRO_ERR;
      snprintf(error->msg, SIMPLET_MAX_ERROR, "cairo error: %s", msg);
      break;
    case SIMPLET_OGR_ERR:
      error->status = SIMPLET_OGR_ERR;
      if(pthread_mutex_lock(&error_lock) > 0) {
        snprintf(error->msg, SIMPLET_MAX_ERROR, "OGR error");
        break;
      }
      snprintf(error->msg, SIMPLET_MAX_ERROR, "OGR error: %s %s", CPLGetLastErrorMsg(), msg);
      pthread_mutex_unlock(&error_lock);
      break;
    case SIMPLET_INVALID_MAP:
      error->status =  SIMPLET_INVALID_MAP;
      snprintf(error->msg, SIMPLET_MAX_ERROR, "Invalid map: %s", msg);
      break;
    case SIMPLET_OK:
      error->status = SIMPLET_OK;
      snprintf(error->msg, SIMPLET_MAX_ERROR, "%s", msg);
  }
}

