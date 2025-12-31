#ifndef CARD_STORAGE_H
#define CARD_STORAGE_H

#include <stdlib.h>
#include <debug.h>
#include <time.h>
#include <string.h>
#include <ti/getcsc.h>

#define CARD_SUIT_MASK 0b11
#define CARD_SPADES 0
#define CARD_HEARTS 1
#define CARD_DIAMONDS 2
#define CARD_CLUBS 3
#define CARD_VALUE_MASK 0b1111 << 2
#define CARD_A 0 << 2
#define CARD_2 1 << 2
#define CARD_3 2 << 2
#define CARD_4 3 << 2
#define CARD_5 4 << 2
#define CARD_6 5 << 2
#define CARD_7 6 << 2
#define CARD_8 7 << 2
#define CARD_9 8 << 2
#define CARD_10 9 << 2
#define CARD_J 10 << 2
#define CARD_Q 11 << 2
#define CARD_K 12 << 2
#define CARD_FACING_MASK 0b1 << 6
#define CARD_FACEUP 0
#define CARD_FACEDOWN 0b1 << 6
#define CARD_INVALID 255

#define CARD_WIDTH 39
#define CARD_HEIGHT 55

typedef uint8_t Card;

typedef unsigned int u24;

typedef struct {
	uint8_t capacity;
	uint8_t usage;
	Card* data;
} CardStorage;

int cs_resize(CardStorage* cs, uint8_t new_capacity);
int cs_add_card(CardStorage* cs, Card new_card);
Card cs_take_card(CardStorage* cs, uint8_t index);
Card cs_take_top_card(CardStorage* cs);
int cs_shuffle(CardStorage* cs);
void cs_debug_print(CardStorage* cs);

#endif