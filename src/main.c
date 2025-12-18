#include <ti/getcsc.h>
#include <graphx.h>
#include <keypadc.h>

#include "gfx/gfx.h"

#include <debug.h>

#include <string.h>

#define CARD_SUIT_MASK 0b11
#define CARD_CLUBS 3
#define CARD_DIAMONDS 2
#define CARD_HEARTS 1
#define CARD_SPADES 0
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
#define CARD_FACING_MASK 1 << 6
#define CARD_FACEUP 0
#define CARD_FACEDOWN 1 << 6

#define DECK 0
#define DISCARD 1
#define COLUM 2
#define SCORING 9

#define GAME_STATE_SELECT_SOURCE = 0
#define GAME_STATE_SELECT_TARGET = 1


#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define bound(a, b, c) (max(b, min(a, c)))

gfx_sprite_t** glifs_flipped;
gfx_sprite_t* glifs[] = {
	red_a,
	red_2,
	red_3,
	red_4,
	red_5,
	red_6,
	red_7,
	red_8,
	red_9,
	red_10,
	red_j,
	red_q,
	red_k,
	orange_a,
	orange_2,
	orange_3,
	orange_4,
	orange_5,
	orange_6,
	orange_7,
	orange_8,
	orange_9,
	orange_10,
	orange_j,
	orange_q,
	orange_k,
	blue_a,
	blue_2,
	blue_3,
	blue_4,
	blue_5,
	blue_6,
	blue_7,
	blue_8,
	blue_9,
	blue_10,
	blue_j,
	blue_q,
	blue_k,
	green_a,
	green_2,
	green_3,
	green_4,
	green_5,
	green_6,
	green_7,
	green_8,
	green_9,
	green_10,
	green_j,
	green_q,
	green_k,
};

typedef uint8_t u8;
typedef int8_t i8;
typedef unsigned int u24;
typedef int i24;
typedef u8 Card;

typedef struct {
	u8 capacity;
	u8 usage;
	Card* data;
} CardStorage;

typedef struct {
	u8 gamestate;
	u8 source_storage;
	u8 source_index;
	u8 target_storage;
	u24 uptime;
	CardStorage storages[13];
} State;

int step(State* state);
void draw(State* state);
int init_gamestate(State* state);	

	//glif_sprites = malloc(sizeof(gfx_rletsprite_t) * 13 * 4 * 2);
	//glif_sprites[GLIF_A | GLIF_CLUBS] = gfx_ConvertMallocRLETSprite(red_a);


u24 terriblerand() {
	static u24 state = 0b100011011110111111001100;
	state += 0b101111100110101100001111;
	state ^= 0b110000101111100101000011;
	state += 0b111100110100001001111110;
	state ^= 0b011000101011101010111111;
	return state;
}

int CardStorage_resize(CardStorage* cs, u8 new_capacity) {
	if (new_capacity < cs->usage)
		return 1;
	Card* temp_pointer = malloc(sizeof(Card) * new_capacity);
	if (!temp_pointer)
		return 1;
	memmove(temp_pointer, cs->data, sizeof(Card) * cs->usage);
	free(cs->data);
	cs->data = temp_pointer;
	cs->capacity = new_capacity;
	return 0;
}

int CardStorage_add_card(CardStorage* cs, Card new_card) {
	if (cs->usage + 1 > cs->capacity)
		return 1;
	cs->data[cs->usage] = new_card;
	++cs->usage;
	return 0;
}

Card CardStorage_take_card(CardStorage* cs, u8 index) {
	if (index >= cs->usage)
		return -1;
	Card out = cs->data[index];
	memmove(&cs->data[index], &cs->data[index + 1], sizeof(Card) * ((cs->usage - 1) - index));
	--cs->usage;
	return out;
}

Card CardStorage_take_top_card(CardStorage* cs) {
	if (cs->usage == 0)
		return -1;
	return cs->data[--cs->usage];
}

int CardStorage_shuffle(CardStorage* cs) {
	if (cs->usage < 1)
		return 1;
	for (u8 i = 0; i < 100; ++i) {
		u24 rand1 = terriblerand() % cs->usage;
		u24 rand2 = terriblerand() % cs->usage;
		Card temp = cs->data[rand1];
		cs->data[rand1] = cs->data[rand2];
		cs->data[rand2] = temp;
	}
	return 0;
}

void CardStorage_debug_print(CardStorage* cs) {
	dbg_printf("starage capacity: %d\n", (u24)cs->capacity);
	dbg_printf("starage usage: %d\n", (u24)cs->usage);
	for (u8 i = 0; i < cs->usage; ++i) {
		dbg_printf("card: %d\n", (u24)cs->data[i]);
	}
	//	dbg_printf("card value: %d, card suit: %d, full card: %d\n", ((u24)card & 0b111100) >> 2, (u24)card & 0b11, (u24)card);
}

int main(void)
{
	gfx_sprite_t* temp1[] = {
		gfx_RotateSpriteHalf(red_a, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(red_2, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(red_3, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(red_4, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(red_5, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(red_6, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(red_7, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(red_8, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(red_9, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(red_10, gfx_MallocSprite(8, 10)),
		gfx_RotateSpriteHalf(red_j, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(red_q, gfx_MallocSprite(7, 10)),
		gfx_RotateSpriteHalf(red_k, gfx_MallocSprite(6, 10)),

		gfx_RotateSpriteHalf(orange_a, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(orange_2, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(orange_3, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(orange_4, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(orange_5, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(orange_6, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(orange_7, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(orange_8, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(orange_9, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(orange_10, gfx_MallocSprite(8, 10)),
		gfx_RotateSpriteHalf(orange_j, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(orange_q, gfx_MallocSprite(7, 10)),
		gfx_RotateSpriteHalf(orange_k, gfx_MallocSprite(6, 10)),

		gfx_RotateSpriteHalf(blue_a, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(blue_2, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(blue_3, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(blue_4, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(blue_5, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(blue_6, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(blue_7, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(blue_8, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(blue_9, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(blue_10, gfx_MallocSprite(8, 10)),
		gfx_RotateSpriteHalf(blue_j, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(blue_q, gfx_MallocSprite(7, 10)),
		gfx_RotateSpriteHalf(blue_k, gfx_MallocSprite(6, 10)),

		gfx_RotateSpriteHalf(green_a, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(green_2, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(green_3, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(green_4, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(green_5, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(green_6, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(green_7, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(green_8, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(green_9, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(green_10, gfx_MallocSprite(8, 10)),
		gfx_RotateSpriteHalf(green_j, gfx_MallocSprite(6, 10)),
		gfx_RotateSpriteHalf(green_q, gfx_MallocSprite(7, 10)),
		gfx_RotateSpriteHalf(green_k, gfx_MallocSprite(6, 10)),
	};
	glifs_flipped = temp1;

	State state = {
		0, // gamestate
		0, // source_storage
		0, // source_index
		0, // target_storage
		0, // uptime
		{ // storages
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
		},
	};
	init_gamestate(&state);

	gfx_Begin();
	gfx_SetPalette(global_palette, sizeof_global_palette, 0);
	gfx_SetTransparentColor(1);
	gfx_SetTextFGColor(2);
    gfx_SetTextBGColor(3);
    gfx_SetDrawBuffer();

    while (step(&state) != sk_Clear)
    {
        draw(&state);
        gfx_SwapDraw();
    }
    gfx_End();

    return 0;
}

int init_gamestate(State* state) {
	dbg_printf("initing\n");
	// populate the deck with one of each card
	CardStorage_resize(&state->storages[DECK], 52);
	CardStorage_resize(&state->storages[DISCARD], 52);
	for (u8 i = 0; i < 52; ++i) {
		CardStorage_add_card(&state->storages[DECK], i | CARD_FACEDOWN); 
	}
	CardStorage_shuffle(&state->storages[DECK]);
	CardStorage_debug_print(&state->storages[DECK]);

	// initial deal
	for (u8 i = 0; i < 7; ++i) {
		CardStorage_resize(&state->storages[COLUM + i], 15);
	}
	for (u8 i = 0; i < 7; ++i) {
		for (u8 j = i; j < 7; ++j) {
			Card card = CardStorage_take_top_card(&state->storages[DECK]);
			if (i == j) {
				card ^= CARD_FACEDOWN;
			}
			else {
				card |= CARD_FACEDOWN;
			}
			CardStorage_add_card(&state->storages[COLUM + j], card);
		}
	}

	return 0;
}

// main looping logic
int step(State* state)
{
	(state->uptime) += 1;

	uint8_t key;
	key = os_GetCSC();

	if (state->gamestate == 0) {
		switch (key) {
			case sk_Right:
			if (state->source_storage < 12) {
				++state->source_storage;
				state->source_index = state->storages[state->source_storage].usage - 1;
			}
			break;
			case sk_Left:
			if (state->source_storage > 0) {
				--state->source_storage;
				state->source_index = state->storages[state->source_storage].usage - 1;
			}
			break;
			case sk_Up:
			if (state->source_index < state->storages[state->source_storage].usage - 1) {
				++state->source_index;
			}
			break;
			case sk_Down:
			if (state->source_index > 0) {
				--state->source_index;
			}
			break;
			case sk_Enter:
			if (state->source_index != 255) {
				state->target_storage = state->source_storage;
				state->gamestate = 1;
			}
			break;
		}
	}
	else if (state->gamestate == 1) {
		switch (key) {
			case sk_Right:
			if (state->target_storage < 12) {
				++state->target_storage;
			}
			break;
			case sk_Left:
			if (state->target_storage > 0) {
				--state->target_storage;
			}
			break;
			case sk_Enter:
			if (state->source_index != 255) {
				Card card = CardStorage_take_card(&state->storages[state->source_storage], state->source_index);
				CardStorage_add_card(&state->storages[state->target_storage], card);
				state->gamestate = 0;
				state->source_storage = 0;
				state->source_index = 0;
				state->target_storage = 0;
			}
			break;
		}
	}
    return key;
}

int drawCard(Card card, int x, int y, bool highlight) {
	if ((card & CARD_FACING_MASK) == CARD_FACEDOWN) {
		gfx_TransparentSprite(bard_back, x, y);
	}
	else {
		gfx_TransparentSprite(blank_card, x, y);
		int glif1_x = x + 2;
		int glif1_y = y + 2;
		int glif2_x = x + 32; // 40 - 2 - 6
		int glif2_y = y + 44; // 56 - 10 - 2
		if ((card & CARD_VALUE_MASK) == CARD_10)
			glif2_x -= 2;
		if ((card & CARD_VALUE_MASK) == CARD_Q)
			glif2_x -= 1;
		gfx_TransparentSprite(glifs[card], glif1_x, glif1_y);
		gfx_TransparentSprite(glifs_flipped[card], glif2_x, glif2_y);
		dbg_printf("card value: %d, card suit: %d, full card: %d\n", ((u24)card & CARD_VALUE_MASK) >> 2, (u24)card & 0b11, (u24)card);
	}
	if (highlight) {
		gfx_SetTextScale(1, 1);
		gfx_PrintStringXY("Selected", x, y);
	}
	return 0;
}

// draw graphics
void draw(State* state)
{
	gfx_ZeroScreen();
	// playfield
	for (u8 colum = 0; colum < 7; ++colum) {
		for (u8 j = 0; j < state->storages[COLUM + colum].usage; ++j) {
			drawCard(
				state->storages[COLUM + colum].data[j],
				colum * 46, j * 17 + 60,
				(COLUM + colum) == state->source_storage && j == state->source_index);
		}
	}

	// deck and discard
	u8 num_to_draw = min(3, state->storages[DECK].usage);
	for (u8 i = 0; i < num_to_draw; ++i) {
		drawCard(state->storages[DECK].data[state->storages[DECK].usage - 1 - i], 2 + i * 16, 2, false);
	}
	num_to_draw = min(3, state->storages[DISCARD].usage);
	for (u8 i = 0; i < num_to_draw; ++i) {
		drawCard(state->storages[DISCARD].data[state->storages[DECK].usage - 1 - i], 80 + i * 16, 2, false);
	}

	gfx_SetTextScale(2, 2);
	gfx_PrintStringXY("sel_sto:", 0, 192);
	gfx_PrintInt(state->source_storage, 2);
	gfx_PrintStringXY("sel_index:", 0, 208);
	gfx_PrintInt(state->source_index, 2);
	gfx_PrintStringXY("tar_sto:", 0, 224);
	gfx_PrintInt(state->target_storage, 2);
	gfx_PrintString(" state:");
	gfx_PrintInt(state->gamestate, 2);

}

