#include <ti/getcsc.h>
#include <graphx.h>

#include "gfx/gfx.h"

#include <debug.h>

#include <string.h>

#define CARD_FACEUP 1 << 6
#define CARD_FACEDOWN 0

#define CARD_SUIT_CLUBS 0b11 << 4
#define CARD_SUIT_DIAMONDS 0b10 << 4
#define CARD_SUIT_HEARTS 0b01 << 4
#define CARD_SUIT_SPADES 0b0 << 4
#define CARD_INVALID 255

#define CARD_ACE 1
#define CARD_2 2
#define CARD_3 3
#define CARD_4 4
#define CARD_5 5
#define CARD_6 6
#define CARD_7 7
#define CARD_8 8
#define CARD_9 9
#define CARD_10 10
#define CARD_JACK 11
#define CARD_QUEEN 12
#define CARD_KING 13
#define CARD_JOKER 14


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
	u24 uptime;
	CardStorage deck;
	CardStorage playfield;
	CardStorage scoring;
} State;

int step(State* state);
void draw(State* state);

gfx_sprite_t* red_a_flipped;
gfx_sprite_t* red_2_flipped;
gfx_sprite_t* red_3_flipped;
gfx_sprite_t* red_4_flipped;
gfx_sprite_t* red_5_flipped;
gfx_sprite_t* red_6_flipped;
gfx_sprite_t* red_7_flipped;
gfx_sprite_t* red_8_flipped;
gfx_sprite_t* red_9_flipped;
gfx_sprite_t* red_10_flipped;
gfx_sprite_t* red_j_flipped;
gfx_sprite_t* red_q_flipped;
gfx_sprite_t* red_k_flipped;


int copy_sprites_to_rotated() {
	red_a_flipped = gfx_RotateSpriteHalf(red_a, gfx_MallocSprite(6, 10));
	red_2_flipped = gfx_RotateSpriteHalf(red_2, gfx_MallocSprite(6, 10));
	red_3_flipped = gfx_RotateSpriteHalf(red_3, gfx_MallocSprite(6, 10));
	red_4_flipped = gfx_RotateSpriteHalf(red_4, gfx_MallocSprite(6, 10));
	red_5_flipped = gfx_RotateSpriteHalf(red_5, gfx_MallocSprite(6, 10));
	red_6_flipped = gfx_RotateSpriteHalf(red_6, gfx_MallocSprite(6, 10));
	red_7_flipped = gfx_RotateSpriteHalf(red_7, gfx_MallocSprite(6, 10));
	red_8_flipped = gfx_RotateSpriteHalf(red_8, gfx_MallocSprite(6, 10));
	red_9_flipped = gfx_RotateSpriteHalf(red_9, gfx_MallocSprite(6, 10));
	red_10_flipped=gfx_RotateSpriteHalf(red_10, gfx_MallocSprite(8, 10));
	red_j_flipped = gfx_RotateSpriteHalf(red_j, gfx_MallocSprite(6, 10));
	red_q_flipped = gfx_RotateSpriteHalf(red_q, gfx_MallocSprite(7, 10));
	red_k_flipped = gfx_RotateSpriteHalf(red_k, gfx_MallocSprite(6, 10));

	return 0;
}

int free_sprites_rotated() {
	free(red_a_flipped);
	free(red_2_flipped);
	free(red_3_flipped);
	free(red_4_flipped);
	free(red_5_flipped);
	free(red_6_flipped);
	free(red_7_flipped);
	free(red_8_flipped);
	free(red_9_flipped);
	free(red_10_flipped);
	free(red_j_flipped);
	free(red_q_flipped);
	free(red_k_flipped);
	return 0;
}

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

Card CardStorage_draw_card(CardStorage* cs, u8 index) {
	if (index >= cs->usage)
		return CARD_INVALID;
	Card out = cs->data[index];
	memmove(&cs->data[index], &cs->data[index + 1], sizeof(Card) * ((cs->usage - 1) - index));
	--cs->usage;
	return out;
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
		dbg_printf("card: %d\n", (u24)cs->data[i] & 0b00001111);
	}
}

int main(void)
{
	State state = {
		0,
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0},
	};

	gfx_Begin();
	copy_sprites_to_rotated();
	gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetDrawBuffer();

	CardStorage_resize(&state.playfield, 52);
	CardStorage_resize(&state.deck, 52);
	for (u8 i = 0; i < 52; ++i) {
		Card generated_card = CARD_FACEUP;
		if (i < 13)
			generated_card |= CARD_SUIT_SPADES;
		else if (i < 26)
			generated_card |= CARD_SUIT_HEARTS;
		else if (i < 39)
			generated_card |= CARD_SUIT_DIAMONDS;
		else
			generated_card |= CARD_SUIT_CLUBS;
		generated_card |= (i % 13) + 1;
		CardStorage_add_card(&state.deck, generated_card); 
	}

	CardStorage_shuffle(&state.deck);
	CardStorage_debug_print(&state.deck);

    while (step(&state))
    {
        draw(&state);
        gfx_SwapDraw();
    }
	free_sprites_rotated();
    gfx_End();

    return 0;
}

// main looping logic
int step(State* state)
{
	if (state->playfield.usage < 14) {
		Card drawncard = CardStorage_draw_card(&state->deck, state->deck.usage - 1);
		CardStorage_add_card(&state->playfield, drawncard);
	}
	(state->uptime) += 1;
    return !os_GetCSC();
}

int drawCard(Card card, int x, int y) {
	gfx_TransparentSprite(blank_card, x, y);
	// flipped because sprite flipping is weird
	int glif1_x = x + (40 - 2 - 6);
	int glif1_y = y + (56 - 10 - 2);
	int glif2_x = x + 2;
	int glif2_y = y + 2;
	switch (card & 0b00001111) {
		case CARD_ACE:
			gfx_TransparentSprite(red_a, glif1_x, glif1_y);
			gfx_TransparentSprite(red_a_flipped, glif2_x, glif2_y);
			break;
		case CARD_2:
			gfx_TransparentSprite(red_2, glif1_x, glif1_y);
			gfx_TransparentSprite(red_2_flipped, glif2_x, glif2_y);
			break;
		case CARD_3:
			gfx_TransparentSprite(red_3, glif1_x, glif1_y);
			gfx_TransparentSprite(red_3_flipped, glif2_x, glif2_y);
			break;
		case CARD_4:
			gfx_TransparentSprite(red_4, glif1_x, glif1_y);
			gfx_TransparentSprite(red_4_flipped, glif2_x, glif2_y);
			break;
		case CARD_5:
			gfx_TransparentSprite(red_5, glif1_x, glif1_y);
			gfx_TransparentSprite(red_5_flipped, glif2_x, glif2_y);
			break;
		case CARD_6:
			gfx_TransparentSprite(red_6, glif1_x, glif1_y);
			gfx_TransparentSprite(red_6_flipped, glif2_x, glif2_y);
			break;
		case CARD_7:
			gfx_TransparentSprite(red_7, glif1_x, glif1_y);
			gfx_TransparentSprite(red_7_flipped, glif2_x, glif2_y);
			break;
		case CARD_8:
			gfx_TransparentSprite(red_8, glif1_x, glif1_y);
			gfx_TransparentSprite(red_8_flipped, glif2_x, glif2_y);
			break;
		case CARD_9:
			gfx_TransparentSprite(red_9, glif1_x, glif1_y);
			gfx_TransparentSprite(red_9_flipped, glif2_x, glif2_y);
			break;
		case CARD_10:
			gfx_TransparentSprite(red_10, glif1_x, glif1_y);
			gfx_TransparentSprite(red_10_flipped, glif2_x, glif2_y);
			break;
		case CARD_JACK:
			gfx_TransparentSprite(red_j, glif1_x, glif1_y);
			gfx_TransparentSprite(red_j_flipped, glif2_x, glif2_y);
			break;
		case CARD_QUEEN:
			gfx_TransparentSprite(red_q, glif1_x, glif1_y);
			gfx_TransparentSprite(red_q_flipped, glif2_x, glif2_y);
			break;
		case CARD_KING:
			gfx_TransparentSprite(red_k, glif1_x, glif1_y);
			gfx_TransparentSprite(red_k_flipped, glif2_x, glif2_y);
			break;
		default:
			dbg_printf("card: %d\n", (u24)card & 0b00001111);
			return 1;
	}
	return 0;
}

// draw graphics
void draw(State* state)
{
	gfx_FillScreen(1);
	for (u8 i = 0; i < state->playfield.usage; ++i) {
		drawCard(state->playfield.data[i], (i % 7) * 46, (i / 7) * 58);
	}
}

