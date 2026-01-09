#ifndef GPFX
#define GPFX


#include <ti/getcsc.h>

// destination, data mask as array of bytes, number of bytes to draw in top, color in botom
void gpfx_monoMaskSprite(uint8_t* screen_pointer, unsigned int height_color, const uint8_t* const glif);

// destination, data mask as array of bytes, number of bytes to draw in top, color in botom
void gpfx_monoMaskSprite_flipped(uint8_t* screen_pointer, unsigned int height_color, const uint8_t* const glif);

#endif