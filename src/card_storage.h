#ifndef CARD_STORAGE_H
#define CARD_STORAGE_H

#define CARD_STORAGE_CAPACITY 52

#define CARD_WIDTH 39
#define CARD_HEIGHT 55

#include <ti/vars.h>
#include <stdbool.h>
#include "card.h"

typedef struct {
	uint8_t usage;
	bool requires_redraw;
	struct Card data[CARD_STORAGE_CAPACITY];
} CardStorage;

void cs_zero(CardStorage* cs);
bool cs_shuffle(CardStorage* cs);
void cs_debug_print(CardStorage* cs);
// moves cards, number = 0 means many


bool cs_insert_card(CardStorage* cs, struct Card new_card, uint8_t index);
bool cs_insert_to_top_card(CardStorage* cs, struct Card new_card);
bool cs_take_card(CardStorage* cs, uint8_t index, struct Card* out);
bool cs_take_top_card(CardStorage* cs, struct Card* out);

bool cs_move_card(
	CardStorage* source_storage,
	CardStorage* target_storage,
	uint8_t source_index,
	uint8_t target_index
);
bool cs_move_cards(
	CardStorage* source_storage,
	CardStorage* target_storage,
	uint8_t source_index,
	uint8_t target_index,
	uint8_t number
);
bool cs_move_top_card(
	CardStorage* source_storage,
	CardStorage* target_storage,
	uint8_t target_index
);
bool cs_move_top_cards(
	CardStorage* source_storage,
	CardStorage* target_storage,
	uint8_t target_index,
	uint8_t number
);
bool cs_move_to_top_card(
	CardStorage* source_storage,
	CardStorage* target_storage,
	uint8_t source_index
);
bool cs_move_to_top_cards(
	CardStorage* source_storage,
	CardStorage* target_storage,
	uint8_t source_index,
	uint8_t number
);
bool cs_move_top_card_to_top(
	CardStorage* source_storage,
	CardStorage* target_storage
);
bool cs_move_top_cards_to_top(
	CardStorage* source_storage,
	CardStorage* target_storage,
	uint8_t number
);

#endif