#include "card_storage.h"

#include <debug.h>
#include <string.h>
#include <ti/vars.h>
#include <stdlib.h>

#include "card.h"
#include "animation.h"

void cs_clear(CardStorage* cs) {
	dbg_printf("cs_clear() on %d\n", (int)cs);
	cs->redraw_frames = 1;
	cs->usage = 0;
	memset(&cs->data[0], 0, sizeof(struct Card) * CARD_STORAGE_CAPACITY);
}

bool cs_shuffle(CardStorage* cs) {
	if (cs->usage <= 1) {
		dbg_printf("Error in cs_shuffle(), no cards to shuffle.\n");
		return false;
	}
	if (cs->redraw_frames == 0) cs->redraw_frames = 1;
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
	if (cs->redraw_frames == 0) cs->redraw_frames = 1;
	dbg_printf("starage usage: %d\n", (int)cs->usage);
	for (uint8_t i = 0; i < cs->usage; ++i) {
		card_dbg_print(cs->data[i]);
	}
}

bool cs_insert_card(CardStorage* cs, struct Card new_card, uint8_t index) {
	dbg_printf("called cs_insert_card()\n");
	if (cs->usage + 1 > CARD_STORAGE_CAPACITY || card_is_invalid(new_card)) {
		dbg_printf("Error in cs_add_card(), card invalid or card won't fit.\n");
		return false;
	}
	if (cs->redraw_frames == 0) cs->redraw_frames = 1;
	memmove(cs->data + index + 1, cs->data + index, sizeof(struct Card) * (cs->usage - index));
	++cs->usage;
	cs->data[index] = new_card;
	return true;
}

bool cs_insert_to_top_card(CardStorage* cs, struct Card new_card) {
	if (cs->usage + 1 > CARD_STORAGE_CAPACITY) {
		dbg_printf("Error in cs_insert_to_top_card(), card won't fit.\n");
		return false;
	}
	if (card_is_invalid(new_card)) {
		dbg_printf("Error in cs_insert_to_top_card(), card invalid.\n");
		return false;
	}
	if (cs->redraw_frames == 0) cs->redraw_frames = 1;
	cs->data[cs->usage] = new_card;
	++cs->usage;
	return true;
}

bool cs_take_card(CardStorage* cs, uint8_t index, struct Card* out) {
	dbg_printf("called cs_take_card()\n");
	if (index >= cs->usage) {
		dbg_printf("Error in cs_take_card(), index (%d) out of bounds of %d.\n", index, cs->usage);
		return false;
	}
	if (cs->redraw_frames == 0) cs->redraw_frames = 1;
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
	if (cs->redraw_frames == 0) cs->redraw_frames = 1;
	--cs->usage;
	*out = cs->data[cs->usage];
	return true;
}

bool cs_move_card(
	CardStorage* source_storage,
	CardStorage* target_storage,
	uint8_t source_index,
	uint8_t target_index,
	uint8_t lifetime
) {
	dbg_printf("called cs_move_card()\n");
	bool was_success = true;
	struct Card card;
	was_success &= cs_take_card(source_storage, source_index, &card);
	card.life_remaining = lifetime;
	was_success &= cs_insert_card(target_storage, card, target_index);
	return was_success;
}

bool cs_move_cards(
	CardStorage* source_storage,
	CardStorage* target_storage,
	uint8_t source_index,
	uint8_t target_index,
	uint8_t number,
	uint8_t lifetime
) {
	bool was_success = true;
	if (number == 0) {
		number = source_storage->usage - source_index;
	}
	for (uint8_t i = 0; i < number; ++i) {
		was_success &= cs_move_card(source_storage, target_storage, source_index, target_index + i, lifetime);
	}
	return was_success;
}

bool cs_move_top_card_to_top(
	CardStorage* source_storage,
	CardStorage* target_storage,
	uint8_t lifetime
) {
	dbg_printf("called cs_move_top_card_to_top()\n");
	bool was_success = true;
	struct Card card;
	was_success &= cs_take_top_card(source_storage, &card);
	card.life_remaining = lifetime;
	was_success &= cs_insert_to_top_card(target_storage, card);
	return was_success;
}
