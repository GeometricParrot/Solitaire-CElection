#ifndef GPFX
#define GPFX


#include <ti/getcsc.h>
#include "card_storage.h"
struct Card;
struct AnimationQueue;

#define buffer_position(x, y) (*(gfx_vbuffer + y) + x)

// destination, data mask as array of bytes, number of bytes to draw in top, color in botom
void gpfx_monoMaskSprite(uint8_t* screen_pointer, unsigned int height_color, const uint8_t* const glif);

// destination, data mask as array of bytes, number of bytes to draw in top, color in botom
void gpfx_monoMaskSprite_flipped(uint8_t* screen_pointer, unsigned int height_color, const uint8_t* const glif);


int gpfx_drawCard(struct Card* card);

void gpfx_draw_highlight(struct Card* card);
// returns true if animated
bool gpfx_draw_maybe_animated_card(struct AnimationQueue* aq, struct Card* card, uint24_t x, uint8_t y, CardStorage* cs);
void gpfx_drawShadowText(const char* str, uint8_t x, uint8_t y, uint8_t scale);




#endif