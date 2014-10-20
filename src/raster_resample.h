#ifndef _SIMPLE_TILES_RASTER_RESAMPLE_H
#define _SIMPLE_TILES_RASTER_RESAMPLE_H

#include <stdint.h>

int
simplet_resample(uint32_t **resampled, uint32_t *data, uint16_t width, uint16_t height);

void*
simplet_grab_gl_context();

void
simplet_destroy_gl_context(void* ctx);

#endif