#ifndef CARD_H
#define CARD_H

#include <ti/vars.h>
#include <stdbool.h>

struct Card {
	uint8_t value;
	// flags def:
	// S: suit
	// F: facing
	// 0000'0FSS
	uint8_t flags;
	uint8_t life_remaining;
	uint8_t target_y;
	uint24_t target_x;
};

enum CARD_SUIT {
	CARD_SUIT_SPADES,
	CARD_SUIT_HEARTS,
	CARD_SUIT_DIAMONDS,
	CARD_SUIT_CLUBS,
};

enum CARD_FACING {
	CARD_FACING_DOWN,
	CARD_FACING_UP,
};

enum CARD_VALUE {
	CARD_VALUE_INVALID,
	CARD_VALUE_ACE,
	CARD_VALUE_2,
	CARD_VALUE_3,
	CARD_VALUE_4,
	CARD_VALUE_5,
	CARD_VALUE_6,
	CARD_VALUE_7,
	CARD_VALUE_8,
	CARD_VALUE_9,
	CARD_VALUE_10,
	CARD_VALUE_JACK,
	CARD_VALUE_QUEEN,
	CARD_VALUE_KING,
};

#define card_suit(card) ((card.flags & 0b00000011) >> 0)
#define card_facing(card) ((card.flags & 0b00000100) >> 2)
#define card_is_valid(card) (card.value != CARD_VALUE_INVALID)
#define card_is_invalid(card) (card.value == CARD_VALUE_INVALID)

#define card_set_suit(card, suit) (card.flags = ((card.flags & 0b11111100) | suit))
#define card_set_facing(card, facing) (card.flags = ((card.flags & 0b11111011) | facing << 2))

uint8_t card_opposite_suit(struct Card card);

void card_dbg_print(struct Card card);

#endif