#ifndef BIT_SPRITES_H
#define BIT_SPRITES_H

#include <ti/getcsc.h>

#define CARD_WIDTH 39
#define CARD_HEIGHT 55

#define GLIF_WIDTH 8
#define GLIF_HEIGHT 10

#define GLIF_SMALL_WIDTH 6
#define GLIF_SMALL_HEIGHT 7

#define COLOR_BACKGROUND 0
#define COLOR_TRANSPARENT 1
#define COLOR_VALID_SELECTION 6
#define COLOR_UNKNOWN_SELECTION 7
#define COLOR_INVALID_SELECTION 8
#define COLOR_WHITE 9

extern uint8_t const data[];

extern uint8_t const glif_locations_x[];

extern uint8_t const glif_locations_y[];

#endif
