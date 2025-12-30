#include <ti/getcsc.h>
#include <graphx.h>
#include <keypadc.h>

#include "gfx/gfx.h"
#include "gpfx.h"
#include "bit_sprites.h"

#include <debug.h>

#include <string.h>

#define COLOR_BACKGROUND 0
#define COLOR_TRANSPARENT 1
#define COLOR_VALID_SELECTION 6
#define COLOR_INVALID_SELECTION 7
#define COLOR_WHITE 8

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

#define DECK 0
#define DISCARD 1
#define COLUM 2
#define SCORING 9

#define GAME_STATE_SELECT_SOURCE = 0
#define GAME_STATE_SELECT_TARGET = 1


#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define bound(a, b, c) (max(b, min(a, c)))

#define as_facedown(card) (card | CARD_FACING_MASK)
#define as_faceup(card) (as_facedown(card) ^ CARD_FACING_MASK)
#define set_faceup(card) (card = as_faceup(card))
#define set_facedown(card) (card = as_facedown(card))

#define top_card(index) storages[index].data[storages[index].usage - 1]


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


u8 gamestate = 0;
u8 src_sto = 0;
u8 src_index = 0;
u8 tar_sto = 0;
u24 uptime = 0;
bool valid_target = false;
CardStorage storages[] = {
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
	{0, 0, 0},
};


bool step();
void draw();
int init_gamestate();


u24 terriblerand() {
	static u24 state = 0b100011011110111111001100;
	state += 0b101111100110101100001111;
	state ^= 0b110000101111100101000011;
	state += 0b111100110100001001111110;
	state ^= 0b011000101011101010111111;
	return state;
}

int cs_resize(CardStorage* cs, u8 new_capacity) {
	if (new_capacity < cs->usage) {
		dbg_printf("Error in cs_resize(), new capacity < usage.\n");
		return 1;
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

int cs_add_card(CardStorage* cs, Card new_card) {
	if (cs->usage + 1 > cs->capacity || new_card == CARD_INVALID) {
		dbg_printf("Error in cs_add_card(), card invalid or card won't fit.\n");
		return 1;
	}
	cs->data[cs->usage] = new_card;
	++cs->usage;
	return 0;
}

Card cs_take_card(CardStorage* cs, u8 index) {
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
	for (u8 i = 0; i < 100; ++i) {
		u24 rand1 = terriblerand() % cs->usage;
		u24 rand2 = terriblerand() % cs->usage;
		Card temp = cs->data[rand1];
		cs->data[rand1] = cs->data[rand2];
		cs->data[rand2] = temp;
	}
	return 0;
}

void cs_debug_print(CardStorage* cs) {
	dbg_printf("starage capacity: %d\n", (u24)cs->capacity);
	dbg_printf("starage usage: %d\n", (u24)cs->usage);
	for (u8 i = 0; i < cs->usage; ++i) {
		dbg_printf("card: %d\n", (u24)cs->data[i]);
	}
}

int main(void)
{
	init_gamestate();

	gfx_Begin();
	gfx_SetPalette(global_palette, sizeof_global_palette, 0);
	gfx_SetTransparentColor(COLOR_TRANSPARENT);
    gfx_SetDrawBuffer();


    while (!step())
    {
        draw();

		//gfx_BlitBuffer();
        gfx_SwapDraw();
	}

    gfx_End();

    return 0;
}

int init_gamestate() {
	dbg_printf("initing\n");
	// populate the deck with one of each card
	cs_resize(&storages[DECK], 52);
	cs_resize(&storages[DISCARD], 52);
	for (u8 i = 0; i < 52; ++i) {
		cs_add_card(&storages[DECK], i | CARD_FACEDOWN); 
	}
	cs_shuffle(&storages[DECK]);
	cs_debug_print(&storages[DECK]);

	// initial deal
	for (u8 i = 0; i < 7; ++i) {
		cs_resize(&storages[COLUM + i], 15);
	}
	for (u8 i = 0; i < 7; ++i) {
		for (u8 j = i; j < 7; ++j) {
			Card card = cs_take_top_card(&storages[DECK]);
			if (i == j) {
				card ^= CARD_FACEDOWN;
			}
			else {
				card |= CARD_FACEDOWN;
			}
			cs_add_card(&storages[COLUM + j], card);
		}
	}
	cs_resize(&storages[SCORING], 13);
	cs_resize(&storages[SCORING + 1], 13);
	cs_resize(&storages[SCORING + 2], 13);
	cs_resize(&storages[SCORING + 3], 13);
	src_sto = COLUM;
	src_index = storages[src_sto].usage - 1;

	return 0;
}

void set_source_colum(u8 colum) {
	src_sto = colum;
	src_index = storages[src_sto].usage - 1;
}

u8 pair_suit(u8 suit) {
	switch (suit) {
		case CARD_SPADES:
		return CARD_CLUBS;

		case CARD_CLUBS:
		return CARD_SPADES;

		case CARD_HEARTS:
		return CARD_DIAMONDS;

		case CARD_DIAMONDS:
		return CARD_HEARTS;
	}
	dbg_printf("Error, %d is invalid suit.\n", suit);
	return 0;
}

// main looping logic
bool step()
{
	uint8_t key = os_GetCSC();
	if (key == sk_Clear) {
		return true;
	}
	++uptime;

	switch (key) {
		case sk_Store:
			if (storages[DECK].usage > 0) {
				cs_add_card(
					&storages[DISCARD],
					as_faceup(cs_take_top_card(&storages[DECK]))
				);
			}
			else {
				while (storages[DISCARD].usage > 0) {
					cs_add_card(
						&storages[DECK],
						as_facedown(cs_take_top_card(&storages[DISCARD]))
					);
				}
			}
		break;

		case sk_Mode:
			storages[src_sto].data[src_index] ^= CARD_FACING_MASK;
		break;

		case sk_Del:
		case sk_Alpha:
			gamestate = 0;
			tar_sto = 0;
			while (storages[src_sto].usage == 0) {
				
				++src_sto;
				if (src_sto > 12)
					src_sto %= 13;
			}
			src_index = storages[src_sto].usage - 1;
		break;
	}

	// select source mode
	if (gamestate == 0) {
		switch (key) {
			case sk_Window:
				if (storages[SCORING + 0].usage > 0)
					set_source_colum(SCORING + 0);
			break;

			case sk_Zoom:
				if (storages[SCORING + 1].usage > 0)
					set_source_colum(SCORING + 1);
			break;

			case sk_Trace:
				if (storages[SCORING + 2].usage > 0)
					set_source_colum(SCORING + 2);
			break;

			case sk_Graph:
				if (storages[SCORING + 3].usage > 0)
					set_source_colum(SCORING + 3);
			break;

			case sk_Yequ:
			case sk_0:
				if (storages[1].usage > 0)
					set_source_colum(1);
			break;

			case sk_1:
				if (storages[COLUM + 0].usage > 0)
					set_source_colum(COLUM + 0);
			break;

			case sk_2:
				if (storages[COLUM + 1].usage > 0)
					set_source_colum(COLUM + 1);
			break;

			case sk_3:
				if (storages[COLUM + 2].usage > 0)
					set_source_colum(COLUM + 2);
			break;

			case sk_4:
				if (storages[COLUM + 3].usage > 0)
					set_source_colum(COLUM + 3);
			break;

			case sk_5:
				if (storages[COLUM + 4].usage > 0)
					set_source_colum(COLUM + 4);
			break;

			case sk_6:
				if (storages[COLUM + 5].usage > 0)
					set_source_colum(COLUM + 5);
			break;

			case sk_7:
				if (storages[COLUM + 6].usage > 0)
					set_source_colum(COLUM + 6);
			break;

			case sk_Right:
				++src_sto;
				while (storages[src_sto].usage == 0 || src_sto == 0) {
					++src_sto;
					if (src_sto > 12)
						src_sto %= 13;
				}
				src_index = storages[src_sto].usage - 1;
			break;

			case sk_Left:
				--src_sto;
				while (storages[src_sto].usage == 0 || src_sto == 0) {
					if (src_sto == 0)
						src_sto = 12;
					else
						--src_sto;
				}
				src_index = storages[src_sto].usage - 1;
			break;

			case sk_Down:
			if (src_index < storages[src_sto].usage - 1) {
				++src_index;
			}
			break;

			case sk_Up:
			if (src_index > 0) {
				--src_index;
			}
			break;

			case sk_2nd:
			case sk_Enter:
			if (src_index < storages[src_sto].usage) {
				tar_sto = src_sto;
				gamestate = 1;
			}
			break;
		}
	}
	else if (gamestate == 1) {
		switch (key) {
			case sk_Window:
				tar_sto = SCORING + 0;
			break;

			case sk_Zoom:
				tar_sto = SCORING + 1;
			break;

			case sk_Trace:
				tar_sto = SCORING + 2;
			break;

			case sk_Graph:
				tar_sto = SCORING + 3;
			break;

			case sk_1:
				tar_sto = COLUM + 0;
			break;

			case sk_2:
				tar_sto = COLUM + 1;
			break;

			case sk_3:
				tar_sto = COLUM + 2;
			break;

			case sk_4:
				tar_sto = COLUM + 3;
			break;

			case sk_5:
				tar_sto = COLUM + 4;
			break;

			case sk_6:
				tar_sto = COLUM + 5;
			break;

			case sk_7:
				tar_sto = COLUM + 6;
			break;

			case sk_Right:
				do {
					++tar_sto;
					if (tar_sto > 12)
						tar_sto %= 13;
					if (tar_sto != 0 && tar_sto != 1)
						break;
				} while (true);
			break;

			case sk_Left:
				do {
					if (tar_sto == 0)
						tar_sto = 12;
					else
						--tar_sto;
					if (tar_sto != 0 && tar_sto != 1)
						break;
				} while (true);
			break;

			case sk_2nd:
			case sk_Enter:
				if (valid_target) {
					do {
						cs_add_card(
							&storages[tar_sto],
							as_faceup(cs_take_card(
								&storages[src_sto],
								src_index
							))
						);
					} while (storages[src_sto].usage > src_index);
					if (storages[src_sto].usage > 0)
						set_faceup(storages[src_sto].data[src_index - 1]);
				}
				gamestate = 0;
				if (tar_sto >= COLUM && tar_sto <= COLUM + 7)
					src_sto = tar_sto;
				src_index = storages[src_sto].usage - 1;
				tar_sto = 0;
			break;
		}
	}
	if (
		tar_sto < COLUM || tar_sto > SCORING + 3
		|| tar_sto == src_sto
	) {
		valid_target = false;
	} else {
		switch (tar_sto) {
			case COLUM + 0:
			case COLUM + 1:
			case COLUM + 2:
			case COLUM + 3:
			case COLUM + 4:
			case COLUM + 5:
			case COLUM + 6:
				// if top card number is +1 and suit is compatable
				dbg_printf("%d\n", (top_card(tar_sto) & CARD_VALUE_MASK));
				if ( // kings on blank spaces
					(storages[src_sto].data[src_index] & CARD_VALUE_MASK) == CARD_K
					&& (storages[tar_sto].usage == 0)
				) {
					valid_target = true;
				} else if (
					(
						(top_card(tar_sto) & CARD_VALUE_MASK) // top target card
						== (storages[src_sto].data[src_index] & CARD_VALUE_MASK) + (1 << 2) // == top source + 1
					) && ( // opposite color suit
						(top_card(tar_sto) & CARD_SUIT_MASK) != (storages[src_sto].data[src_index] & CARD_SUIT_MASK)
						&& (top_card(tar_sto) & CARD_SUIT_MASK) != pair_suit(storages[src_sto].data[src_index] & CARD_SUIT_MASK)
					)
				) {
					valid_target = true;
				}
				else {
					valid_target = false;
				}
			break;

			case SCORING + 0:
			case SCORING + 1:
			case SCORING + 2:
			case SCORING + 3:
				if (
					( // correct suit
						(storages[src_sto].data[src_index] & CARD_SUIT_MASK) == tar_sto - SCORING
					) && (
						(
							storages[tar_sto].usage == 0 // target empty
							&& ((storages[src_sto].data[src_index] & CARD_VALUE_MASK) == CARD_A) // source is ace
						) || ( // target used and source == target + 1
							storages[tar_sto].usage > 0 // target has cards
							&& (storages[src_sto].data[src_index] & CARD_VALUE_MASK) // source card && value
								== (top_card(tar_sto) & CARD_VALUE_MASK) + (1 << 2) // == target + 1
						)
					)
				) {
					valid_target = true;
				} else {
					valid_target = false;
				}
			break;

			default:
				valid_target = false;
			break;
		}
	}
    return false;
}

int drawCard(Card card, int x, int y, bool highlight) {
	// facedown
	if ((card & CARD_FACING_MASK) == CARD_FACEDOWN) {
		gfx_TransparentSprite(bard_backv2, x, y);
	}
	else {
		switch (card & CARD_VALUE_MASK) {
			case CARD_K:
			gfx_TransparentSprite(king, x, y);
			break;

			default:
			gfx_TransparentSprite(blank_cardv2, x, y);
		}

		int glif1_x = x + 2;
		int glif1_y = y + 5;
		int glif2_x = x + CARD_WIDTH - 2 - 8;
		int glif2_y = y + CARD_HEIGHT - 10 - 5;
		gpfx_monoMaskSprite(
			*(gfx_vbuffer + glif1_y) + glif1_x,
			(10 << 8) + (2 + (card & CARD_SUIT_MASK)),
			data + 10 * ((card & CARD_VALUE_MASK) >> 2)
		);
		gpfx_monoMaskSprite_flipped(
			*(gfx_vbuffer + glif2_y) + glif2_x,
			(10 << 8) + (2 + (card & CARD_SUIT_MASK)),
			data + 10 * ((card & CARD_VALUE_MASK) >> 2)
		);
		unsigned int height_color = (7 << 8) + (2 + (card & CARD_SUIT_MASK));
		void* glif = data + 130 + 7 * (card & CARD_SUIT_MASK);
		
		u8 value = (card & CARD_VALUE_MASK) >> 2;
		u8 sum = (value*(value+1)) / 2;
		if (value < 10) {
			for (u8 i = 0; i <= value; ++i) {
				gpfx_monoMaskSprite(
					*(gfx_vbuffer + y + glif_locations_y[sum + i]) + x + glif_locations_x[sum + i],
					height_color,
					glif
				);
			}
		}
		else if (value < 13) {
			gpfx_monoMaskSprite(
				*(gfx_vbuffer + y + 5) + x + CARD_WIDTH - 3 - GLIF_SMALL_WIDTH,
				height_color,
				glif
			);
			gpfx_monoMaskSprite_flipped(
				*(gfx_vbuffer + y + CARD_HEIGHT - GLIF_SMALL_HEIGHT - 5) + x + 1,
				height_color,
				glif
			);
		}
	}
	// box to highlight selected card
	if (highlight) {
		gfx_SetColor(COLOR_WHITE);
		gfx_Rectangle(x - 2, y - 2, 44, 60);
	}
	return 0;
}

// draw graphics
void draw()
{
	gfx_ZeroScreen();
	// playfield
	for (u8 colum = 0; colum < 7; ++colum) {
		if (COLUM + colum == tar_sto) {
			if (valid_target)
				gfx_SetColor(COLOR_VALID_SELECTION);
			else
				gfx_SetColor(COLOR_INVALID_SELECTION);
			gfx_Rectangle(colum * 46, 58, 44, 64 + max((storages[COLUM + colum].usage - 1) * 13, 1));
		}
		for (u8 j = 0; j < storages[COLUM + colum].usage; ++j) {
			drawCard(
				storages[COLUM + colum].data[j],
				colum * 46 + 2, j * 13 + 60,
				(COLUM + colum) == src_sto && j == src_index
			);
		}
	}

	// draw the deck
	u8 num_to_draw = min(3, storages[DECK].usage);
	for (u8 i = 0; i < num_to_draw; ++i) {
		drawCard(
			storages[DECK].data[storages[DECK].usage + i - num_to_draw],
			2 + i * 16, 2,
			DECK == src_sto && (storages[DECK].usage + i - num_to_draw) == src_index
		);
	}
	// draw the discard pile
	num_to_draw = min(3, storages[DISCARD].usage);
	for (u8 i = 0; i < num_to_draw; ++i) {
		drawCard(
			storages[DISCARD].data[storages[DISCARD].usage + i - num_to_draw],
			80 + i * 16, 2,
			DISCARD == src_sto && (storages[DISCARD].usage + i - num_to_draw) == src_index
		);
	}
	// draw the scoring piles
	for (u8 i = 0; i < 4; ++i) {
		if (SCORING + i == tar_sto) {
			if (valid_target)
				gfx_SetColor(COLOR_VALID_SELECTION);
			else
				gfx_SetColor(COLOR_INVALID_SELECTION);
			gfx_Rectangle_NoClip(150 + i * 42, 0, CARD_WIDTH + 4, CARD_HEIGHT + 4);
		}
		if (storages[SCORING + i].usage > 0) {
			drawCard(
				top_card(SCORING + i),
				150 + i * (CARD_WIDTH + 4), 2,
				SCORING + i == src_sto && storages[SCORING + i].usage - 1 == src_index
			);
		}
		else {
			gpfx_monoMaskSprite(
			*(gfx_vbuffer + CARD_HEIGHT/2 - 4) + 150 + CARD_WIDTH/2 - 4 + 1 + i * (CARD_WIDTH + 4),
			(7 << 8) + (5),
			data + 130 + 7 * (i)
		);
		}
	}
}