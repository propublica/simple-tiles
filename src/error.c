#include <cpl_error.h>
#include <cpl_conv.h>
#include "error.h"

// Add a bit of debugging information to the error.
int
simplet_set_error(simplet_errorable_t *error, simplet_status_t status, const char *msg){
  int res = 1;
  switch(status){
    case SIMPLET_ERR:
      error->status = SIMPLET_ERR;
      res = asprintf(&error->error_msg, "simple tiles error: %s", msg);
      break;
    case SIMPLET_OOM:
      error->status = SIMPLET_OOM;
      res = asprintf(&error->error_msg,  "out of memory for allocation, %s", msg);
      break;
    case SIMPLET_CAIRO_ERR:
      error->status = SIMPLET_CAIRO_ERR;
      res = asprintf(&error->error_msg, "cairo error: %s", msg);
      break;
    case SIMPLET_OGR_ERR:
      error->status = SIMPLET_OGR_ERR;
      res = asprintf(&error->error_msg, "OGR error: %s, %s", CPLGetLastErrorMsg(), msg);
      break;
    case SIMPLET_GDAL_ERR:
      error->status = SIMPLET_GDAL_ERR;
      res = asprintf(&error->error_msg, "GDAL error: %s, %s", CPLGetLastErrorMsg(), msg);
      break;
    case SIMPLET_OK:
      error->status = SIMPLET_OK;
      res = asprintf(&error->error_msg, "%s", msg);
  }
  return res;
}

// Set error on an errorable.
simplet_status_t
simplet_error(simplet_errorable_t *errr, simplet_status_t err, const char *msg){
  simplet_set_error(errr, err, msg);
  return err;
}

