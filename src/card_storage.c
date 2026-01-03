#include "card_storage.h"

int cs_resize(CardStorage* cs, uint8_t new_capacity) {
	if (new_capacity < cs->usage) {
		dbg_printf("Error in cs_resize(), new capacity < usage.\n");
		return 1;
	}
	if (new_capacity == cs->capacity) {
		dbg_printf("maybe unexpected in cs_resize(), new capacity = capacity.\n");
		return 0;
	}
	Card* temp_pointer = malloc(sizeof(Card) * new_capacity);
	if (!temp_pointer)
		return 1;
	memmove(temp_pointer, cs->data, sizeof(Card) * cs->usage);
	free(cs->data);
	cs->data = temp_pointer;
	cs->capacity = new_capacity;
	return 0;
}

void cs_reset(CardStorage* cs) {
	cs->capacity = 0;
	cs->usage = 0;
	if (cs->data)
		free(cs->data);
	cs->data = NULL;
}

int cs_add_card(CardStorage* cs, Card new_card) {
	if (cs->usage + 1 > cs->capacity || new_card == CARD_INVALID) {
		dbg_printf("Error in cs_add_card(), card invalid or card won't fit.\n");
		return 1;
	}
	cs->data[cs->usage] = new_card;
	++cs->usage;
	return 0;
}

Card cs_take_card(CardStorage* cs, uint8_t index) {
	if (index >= cs->usage) {
		dbg_printf("Error in cs_take_card(), index (%d) out of bounds.\n", index);
		return CARD_INVALID;
	}
	Card out = cs->data[index];
	memmove(&cs->data[index], &cs->data[index + 1], sizeof(Card) * ((cs->usage - 1) - index));
	--cs->usage;
	return out;
}

Card cs_take_top_card(CardStorage* cs) {
	if (cs->usage == 0) {
		dbg_printf("Error in cs_take_top_card(), no cards to take.\n");
		return CARD_INVALID;
	}
	--cs->usage;
	return cs->data[cs->usage];
}

int cs_shuffle(CardStorage* cs) {
	if (cs->usage < 1) {
		dbg_printf("Error in cs_shuffle(), no cards to shuffle.\n");
		return 1;
	}
	srand(time(NULL));
	for (uint8_t i = 0; i < 255; ++i) {
		u24 rand1 = rand() % cs->usage;
		u24 rand2 = rand() % cs->usage;
		Card temp = cs->data[rand1];
		cs->data[rand1] = cs->data[rand2];
		cs->data[rand2] = temp;
	}
	return 0;
}

void cs_debug_print(CardStorage* cs) {
	dbg_printf("starage capacity: %d\n", (u24)cs->capacity);
	dbg_printf("starage usage: %d\n", (u24)cs->usage);
	for (uint8_t i = 0; i < cs->usage; ++i) {
		dbg_printf("card: %d\n", (u24)cs->data[i]);
	}
}