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
      asprintf(&error->error_msg, "simple tiles error: %s", msg);
      break;
    case SIMPLET_OOM:
      error->status = SIMPLET_OOM;
      asprintf(&error->error_msg,  "out of memory for allocation, %s", msg);
      break;
    case SIMPLET_CAIRO_ERR:
      error->status = SIMPLET_CAIRO_ERR;
      asprintf(&error->error_msg, "cairo error: %s", msg);
      break;
    case SIMPLET_OGR_ERR:
      error->status = SIMPLET_OGR_ERR;
      asprintf(&error->error_msg, "OGR error: %s, %s", CPLGetLastErrorMsg(), msg);
      break;
    case SIMPLET_OK:
      error->status = SIMPLET_OK;
      asprintf(&error->error_msg, "%s", msg);
  }
}

