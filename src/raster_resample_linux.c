#ifdef ST_LINUX
void*
simplet_grab_gl_context(){
  return 0;
}

void
simplet_destroy_gl_context(void* ctx){
  (void)ctx;
}
#endif
