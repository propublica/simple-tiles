#include "raster_resample_port.h"

#ifdef ST_LINUX
#include <stdlib.h>

struct st_ctx {
  uint32_t *data;
  OSMesaContext ctx;
};

void*
simplet_grab_gl_context(uint16_t width, uint16_t height){
  OSMesaContext ctx = OSMesaCreateContextExt(OSMESA_RGBA, 16, 0, 0, NULL);
  if(!ctx) return NULL;

  uint32_t *buf = calloc(width / 2 * height / 2, sizeof(uint32_t));
  if(buf == NULL) goto destroyctx;

  int err = OSMesaMakeCurrent(ctx, buf, GL_UNSIGNED_BYTE, width, height);
  if(err == 0) goto cleanup;

  struct st_ctx *stctx = malloc(sizeof(struct st_ctx));
  if(stctx == NULL) goto cleanup;

  stctx->data = buf;
  stctx->ctx  = ctx;

  return stctx;

cleanup:
  free(buf);
destroyctx:
  OSMesaDestroyContext(ctx);
  return NULL;
}

void
simplet_destroy_gl_context(void* ctx){
  struct st_ctx *stctx = (struct st_ctx *)ctx;
  free(stctx->data);
  OSMesaDestroyContext(stctx->ctx);
  free(stctx);
}
#endif
