#ifdef ST_APPLE
#include "raster_resample_port.h"

void*
simplet_grab_gl_context(){
  CGLContextObj context;
  CGLPixelFormatAttribute attributes[5] = {
    kCGLPFAAccelerated,
    kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
    kCGLPFADoubleBuffer,
    (CGLPixelFormatAttribute) 0
  };
  CGLPixelFormatObj pix;
  CGLError error;
  GLint num;
  error = CGLChoosePixelFormat(attributes, &pix, &num);
  // add error checking here
  error = CGLCreateContext(pix, 0, &context);
  // add error checking here
  CGLDestroyPixelFormat(pix);
  error = CGLSetCurrentContext(context);
  return context;
}

void
simplet_destroy_gl_context(void* ctx){
  CGLSetCurrentContext(0);
  CGLDestroyContext(ctx);
}

#endif
