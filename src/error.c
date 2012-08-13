#include <cpl_error.h>
#include "error.h"


// The OGR handler to silence ogr errors.
static void
ogr_error_handler(CPLErr eclass, int err_no, const char *msg){
  (void)eclass, (void)err_no, (void)msg;
}

// Set the ogr handler.
void
simplet_error_init(){
  CPLSetErrorHandler(ogr_error_handler);
}

// Set error on an errorable.
simplet_status_t
simplet_error(simplet_errorable_t *errr, simplet_status_t err, const char *msg){
  simplet_set_error(errr, err, msg);
  return err;
}

// Add a bit of debugging information to the error.
void
simplet_set_error(simplet_errorable_t *error, simplet_status_t status, const char *msg){
  switch(status){
    case SIMPLET_ERR:
      error->status = SIMPLET_ERR;
      snprintf(error->error_msg, SIMPLET_MAX_ERROR, "simple tiles error: %s", msg);
      break;
    case SIMPLET_OOM:
      error->status = SIMPLET_OOM;
      snprintf(error->error_msg, SIMPLET_MAX_ERROR, "out of memory for allocation, %s", msg);
      break;
    case SIMPLET_CAIRO_ERR:
      error->status = SIMPLET_CAIRO_ERR;
      snprintf(error->error_msg, SIMPLET_MAX_ERROR, "cairo error: %s", msg);
      break;
    case SIMPLET_OGR_ERR:
      error->status = SIMPLET_OGR_ERR;
      snprintf(error->error_msg, SIMPLET_MAX_ERROR, "OGR error: %s, %s", CPLGetLastErrorMsg(), msg);
      break;
    case SIMPLET_OK:
      error->status = SIMPLET_OK;
      snprintf(error->error_msg, SIMPLET_MAX_ERROR, "%s", msg);
  }
}

