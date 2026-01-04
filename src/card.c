#include "card.h"

#include <ti/vars.h>
#include <debug.h>

uint8_t card_opposite_suit(struct Card card) {
	switch (card_suit(card)) {
		case CARD_SUIT_SPADES:
		return CARD_SUIT_CLUBS;

		case CARD_SUIT_HEARTS:
		return CARD_SUIT_DIAMONDS;

		case CARD_SUIT_DIAMONDS:
		return CARD_SUIT_HEARTS;

		case CARD_SUIT_CLUBS:
		return CARD_SUIT_SPADES;
	}
	dbg_printf("Error, nvalid suit.\n");
	return 1;
}

void card_set_equal(struct Card* tar, struct Card src) {
	tar->flags = src.flags;
	tar->value = src.value;
}