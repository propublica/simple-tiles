#include <gdal/cpl_error.h>
#include <pthread.h>
#include "error.h"


static pthread_mutex_t error_lock = PTHREAD_MUTEX_INITIALIZER;

static void
default_error_handler(simplet_status_t err, const char *msg){
  printf("simple tiles error %i: %s\n", err, msg);
}

static simplet_error_handler error_handler = &default_error_handler;
static int error_initialized = 0;

static void
ogr_error_handler(CPLErr eclass, int err_no, const char *msg){
  (void)eclass, (void)err_no; // FIXME
  error_handler(SIMPLET_OGR_ERR, msg);
}

void
simplet_error_init(){
  if(!error_initialized) return;
  pthread_mutex_lock(&error_lock);
  error_initialized = 1; // make threadsafe
  CPLSetErrorHandler(ogr_error_handler);
  pthread_mutex_unlock(&error_lock);
}

simplet_status_t
simplet_error(simplet_status_t err){
  pthread_mutex_lock(&error_lock);
  switch(err){
    case SIMPLET_ERR:
      error_handler(SIMPLET_ERR, "simple tiles error");
      break;
    case SIMPLET_OOM:
      error_handler(SIMPLET_OOM, "out of memory for allocation");
      break;
    case SIMPLET_CAIRO_ERR:
      error_handler(SIMPLET_CAIRO_ERR, "cairo error");
      break;
    case SIMPLET_OGR_ERR:
      error_handler(SIMPLET_OGR_ERR, "OGR error");
      break;
    case SIMPLET_OK:
      return SIMPLET_OK;
  }
  pthread_mutex_unlock(&error_lock);
  return err;
}

simplet_status_t
simplet_check_cairo(cairo_t *ctx){
  cairo_status_t status = cairo_status(ctx);

  if(status == CAIRO_STATUS_SUCCESS)
    return SIMPLET_OK;

  error_handler(SIMPLET_CAIRO_ERR, cairo_status_to_string(status));
  return SIMPLET_ERR;
}

void
simplet_set_error_handle(simplet_error_handler handle){
  error_handler = handle;
}
