#ifndef GPFX
#define GPFX


#include <ti/getcsc.h>

// destination, data mask as array of bytes, number of bytes to draw in top, color in botom
void gpfx_monoMaskSprite(void* buffer, unsigned int height_color, void* data);

// destination, data mask as array of bytes, number of bytes to draw in top, color in botom
void gpfx_monoMaskSprite_flipped(void* buffer, unsigned int height_color, void* data);

#endif