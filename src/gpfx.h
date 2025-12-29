#ifndef GPFX
#define GPFX


#include <ti/getcsc.h>

// destination, data mask as array of bytes, number of bytes to draw in top, color in botom
void gpfx_monoMaskSprite(void* screen_pointer, unsigned int height_color, void* glif);

// destination, data mask as array of bytes, number of bytes to draw in top, color in botom
void gpfx_monoMaskSprite_flipped(void* screen_pointer, unsigned int height_color, void* glif);

#endif