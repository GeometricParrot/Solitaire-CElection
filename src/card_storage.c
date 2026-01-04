#include "card_storage.h"

#include <debug.h>
#include <string.h>
#include <ti/vars.h>
#include <stdlib.h>

#include "card.h"

void cs_zero(CardStorage* cs) {
	//cs->capacity = CARD_STORAGE_CAPACITY;
	cs->usage = 0;
	//cs->data = NULL;
}

bool cs_add_card(CardStorage* cs, struct Card new_card) {
	if (/*cs->usage + 1 > cs->capacity ||*/ card_is_invalid(new_card)) {
		dbg_printf("Error in cs_add_card(), card invalid or card won't fit.\n");
		return false;
	}
	cs->data[cs->usage] = new_card;
	++cs->usage;
	return true;
}

bool cs_insert_card(CardStorage* cs, struct Card new_card, uint8_t index) {
	if (/*cs->usage + 1 > cs->capacity ||*/ card_is_invalid(new_card)) {
		dbg_printf("Error in cs_add_card(), card invalid or card won't fit.\n");
		return false;
	}
	++cs->usage;
	memmove(cs->data + index + 1, cs->data + index, sizeof(struct Card) * (cs->usage - index));
	card_set_equal(cs->data + index, new_card);
	return true;
}

bool cs_take_card(CardStorage* cs, uint8_t index, struct Card* out) {
	if (index >= cs->usage) {
		dbg_printf("Error in cs_take_card(), index (%d) out of bounds of %d.\n", index, cs->usage);
		return false;
	}
	*out = cs->data[index];
	--cs->usage;
	memmove(cs->data + index, cs->data + index + 1, sizeof(struct Card) * (cs->usage - index));
	return true;
}

bool cs_take_top_card(CardStorage* cs, struct Card* out) {
	if (cs->usage == 0) {
		dbg_printf("Error in cs_take_top_card(), no cards to take.\n");
		return false;
	}
	--cs->usage;
	*out = cs->data[cs->usage];
	return true;
}

bool cs_shuffle(CardStorage* cs) {
	if (cs->usage <= 1) {
		dbg_printf("Error in cs_shuffle(), no cards to shuffle.\n");
		return false;
	}
	for (uint8_t i = 0; i < 255; ++i) {
		int rand1 = rand() % cs->usage;
		int rand2 = rand() % cs->usage;
		struct Card temp = cs->data[rand1];
		cs->data[rand1] = cs->data[rand2];
		cs->data[rand2] = temp;
	}
	return true;
}

void cs_debug_print(CardStorage* cs) {
	//dbg_printf("starage capacity: %d\n", (int)cs->capacity);
	dbg_printf("starage usage: %d\n", (int)cs->usage);
	for (uint8_t i = 0; i < cs->usage; ++i) {
		dbg_printf("card: %d of %d\n", cs->data[i].value, cs->data[i].flags);
	}
}

bool cs_move_card(
	CardStorage* source_storage,
	CardStorage* target_storage,
	uint8_t source_index,
	uint8_t target_index
) {
	struct Card card;
	cs_take_card(source_storage, source_index, &card);
	cs_insert_card(target_storage, card, target_index);
	return true;
}

bool cs_move_cards(
	CardStorage* source_storage,
	CardStorage* target_storage,
	uint8_t source_index,
	uint8_t target_index,
	uint8_t number
) {
	if (number == 0) {
		number = source_storage->usage - source_index;
	}
	for (uint8_t i = 0; i < number; ++i) {
		cs_move_card(source_storage, target_storage, source_index, target_index + i);
	}
	return true;
}
