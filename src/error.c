#include "error.h"

static void
default_error_handler(simplet_status_t err, const char *msg){
  printf("simple tiles error %i: %s\n", err, msg);
}

static simplet_error_handler error_handler = &default_error_handler; 

simplet_status_t
simplet_error(simplet_status_t err){
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

// doesn't work yet
//static void
//ogr_error_handler(CPLErr eclass, int err_no, const char *msg){
//  error_handler(SIMPLET_OGR_ERROR, msg);
//}
//CPLSetErrorHandler((CPLErrorHandler)ogr_error_handler);

void
simplet_set_error_handle(simplet_error_handler handle){
  error_handler = handle;
}
