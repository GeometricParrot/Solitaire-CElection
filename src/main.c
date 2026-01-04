#include <ti/getcsc.h>
#include <graphx.h>
#include <keypadc.h>
#include <time.h>
#include <debug.h>

#include "gfx/gfx.h"
#include "gpfx.h"
#include "bit_sprites.h"
#include "card_storage.h"
#include "card.h"

#define COLOR_BACKGROUND 0
#define COLOR_TRANSPARENT 1
#define COLOR_VALID_SELECTION 6
#define COLOR_UNKNOWN_SELECTION 7
#define COLOR_INVALID_SELECTION 8
#define COLOR_WHITE 9

#define DECK 0
#define DISCARD 1
#define COLUM 2
#define SCORING 9


enum GAME_STATE {
	GAME_STATE_NULL,
	GAME_STATE_SELECT_SOURCE,
	GAME_STATE_SELECT_TARGET,
	GAME_STATE_WON,
};

enum PROGRAM_STATE {
	PROGRAM_STATE_NULL,
	PROGRAM_STATE_MAIN_MENU,
	PROGRAM_STATE_KLONDIKE_IN_GAME,
};

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define bound(a, b, c) (max(b, min(a, c)))

#define top_card(index) g_storages[index].data[g_storages[index].usage - 1]

int g_program_state = PROGRAM_STATE_MAIN_MENU;
int g_game_state = GAME_STATE_NULL;
uint8_t g_src_sto = 0;
uint8_t g_src_index = 0;
uint8_t g_tar_sto = 0;
clock_t g_game_start_time = 0;
int g_move_count = 0;
bool g_valid_target = false;
bool g_klondike_run_auto_win = false;
CardStorage g_storages[13];

bool step();
void draw();
int init_klondike();

int main(void)
{
	//init_gamestate();
	srand(time(NULL));

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

int init_klondike() {
	dbg_printf("initing\n");
	// populate the deck with one of each card
	//cs_resize(&g_storages[DECK], 52);
	//cs_resize(&g_storages[DISCARD], 52);
	for (uint8_t i = 0; i < 52; ++i) {
		struct Card card = {(i%13) + 1, i/13};
		cs_add_card(&g_storages[DECK], card);
	}
	cs_shuffle(&g_storages[DECK]);
	cs_debug_print(&g_storages[DECK]);

	// initial deal
	for (uint8_t i = 0; i < 7; ++i) {
		//cs_resize(&g_storages[COLUM + i], i + 13);
	}
	for (uint8_t i = 0; i < 7; ++i) {
		for (uint8_t j = i; j < 7; ++j) {
			struct Card card;
			cs_take_top_card(&g_storages[DECK], &card);
			if (i == j) {
				card_set_facing(card, CARD_FACING_UP);
			}
			else {
				card_set_facing(card, CARD_FACING_DOWN);
			}
			cs_add_card(&g_storages[COLUM + j], card);
		}
	}
	//cs_reset(&g_storages[SCORING]);
	//cs_reset(&g_storages[SCORING + 1]);
	//cs_reset(&g_storages[SCORING + 2]);
	//cs_reset(&g_storages[SCORING + 3]);
//
	//cs_resize(&g_storages[SCORING], 13);
	//cs_resize(&g_storages[SCORING + 1], 13);
	//cs_resize(&g_storages[SCORING + 2], 13);
	//cs_resize(&g_storages[SCORING + 3], 13);
	g_src_sto = COLUM;
	g_src_index = g_storages[g_src_sto].usage - 1;

	return 0;
}

void set_source_colum(uint8_t colum) {
	g_src_sto = colum;
	g_src_index = g_storages[g_src_sto].usage - 1;
}

bool is_valid_klondike_g_tar_sto(uint8_t storage) {
	if (storage < COLUM || storage > SCORING + 3
		|| storage == g_src_sto
	) {
		return false;
	}
	switch (storage) {
		case COLUM + 0:
		case COLUM + 1:
		case COLUM + 2:
		case COLUM + 3:
		case COLUM + 4:
		case COLUM + 5:
		case COLUM + 6:
			// if top card number is +1 and suit is compatable
			if ( // kings on blank spaces
				g_storages[g_src_sto].data[g_src_index].value == CARD_VALUE_KING
				&& (g_storages[storage].usage == 0)
			) {
				return true;
			} else if (
				(
					(top_card(storage).value) == g_storages[g_src_sto].data[g_src_index].value + 1 // == top source + 1
				) && ( // opposite color suit
					card_suit(top_card(storage)) != g_storages[g_src_sto].data[g_src_index].value
					&& card_suit(top_card(storage)) != card_opposite_suit(g_storages[g_src_sto].data[g_src_index])
				)
			) {
				return true;
			}
			else {
				return false;
			}
		break;

		case SCORING + 0:
		case SCORING + 1:
		case SCORING + 2:
		case SCORING + 3:
			if (g_src_index != g_storages[g_src_sto].usage - 1) { // top card is selected
				return false;
			} else if (
				( // correct suit
					card_suit(g_storages[g_src_sto].data[g_src_index]) == storage - SCORING
				) && (
					(
						g_storages[storage].usage == 0 // target empty
						&& (g_storages[g_src_sto].data[g_src_index].value == CARD_VALUE_ACE) // source is ace
					) || ( // target used and source == target + 1
						g_storages[storage].usage > 0 // target has cards
						&& g_storages[g_src_sto].data[g_src_index].value // source card && value
							== top_card(storage).value + 1 // == target + 1
					)
				)
			) {
				return true;
			} else {
				return false;
			}
		break;

		default:
			return false;
		break;
	}
}

bool is_valid_klondike_g_src_index(uint8_t index) {
	if (index >= g_storages[g_src_sto].usage) {
		return false;
	}
	switch (g_src_sto) {
		case DISCARD:
		case SCORING + 0:
		case SCORING + 1:
		case SCORING + 2:
		case SCORING + 3:
			if (index == g_storages[g_src_sto].usage - 1) {
				return true;
			} else {
				return false;
			}
		break;

		case COLUM + 0:
		case COLUM + 1:
		case COLUM + 2:
		case COLUM + 3:
		case COLUM + 4:
		case COLUM + 5:
		case COLUM + 6:
			if (index == g_storages[g_src_sto].usage - 1) {
				return true;
			} else if (card_facing(g_storages[g_src_sto].data[index]) == CARD_FACING_UP) {
				return true;
			} else {
				return false;
			}
		break;
	}
	return false;
}

bool klondike_is_auto_winnable() {
	if (g_storages[DECK].usage != 0)
		return false;
	if (g_storages[DISCARD].usage > 1)
		return false;
	for (uint8_t colum = COLUM; colum < COLUM + 7; ++colum) {
		for (uint8_t card_index = 0; card_index < g_storages[colum].usage; ++ card_index) {
			if (card_facing(g_storages[colum].data[card_index]) == CARD_FACING_DOWN) {
				return false;
			}
		}
	}
	return true;
}

bool klondike_is_won() {
	if (g_storages[DECK].usage != 0)
		return false;
	if (g_storages[DISCARD].usage != 0)
		return false;
	for (uint8_t colum = COLUM; colum < COLUM + 7; ++colum) {
		if (g_storages[colum].usage != 0)
			return false;
	}
	for (uint8_t scoring = SCORING; scoring < SCORING + 4; ++scoring) {
		if (top_card(scoring).value != CARD_VALUE_KING && card_suit(top_card(scoring)) == scoring - SCORING) {
			return false;
		}
	}
	return true;
}

void klondike_step(uint8_t key) {
	if (klondike_is_won()) {
		g_game_state = GAME_STATE_WON;
		dbg_printf("Won the game\n");
		g_klondike_run_auto_win = false;
		return;
	} else if (g_klondike_run_auto_win) {
		for (uint8_t source = DISCARD; source < COLUM + 7; ++source) {
			if (g_storages[source].usage > 0) {
				for (uint8_t target = SCORING; target < SCORING + 4; ++target) {
					if (
						( // correct suit
							card_suit(top_card(source)) == target - SCORING
						) && (
							(
								g_storages[target].usage == 0 // target empty
								&& (top_card(source).value == CARD_VALUE_ACE) // source is ace
							) || ( // target used and source == target + 1
								g_storages[target].usage > 0 // target has cards
								&& top_card(source).value == top_card(target).value + 1 // == target + 1
							)
						)
					) {
						dbg_printf("Moving card\n");
						cs_move_cards(&g_storages[source], &g_storages[target], g_storages[source].usage - 1, g_storages[target].usage, 0);
						return;
					}
				}
			}
		}
		dbg_printf("Something went wrong with the auto win\n");
		g_klondike_run_auto_win = false;
	}
	
	if (g_game_state == GAME_STATE_SELECT_SOURCE || g_game_state == GAME_STATE_SELECT_TARGET) {
		switch (key) {
			case sk_Math:
				if (klondike_is_auto_winnable())
					g_klondike_run_auto_win = true;
			break;
			case sk_Del:
			case sk_Alpha:
				g_game_state = GAME_STATE_SELECT_SOURCE;
				g_tar_sto = 0;
				while (!is_valid_klondike_g_src_index(g_src_index)) {
					++g_src_sto;
					if (g_src_sto > 12)
						g_src_sto %= 13;
					g_src_index = g_storages[g_src_sto].usage - 1;
				}
			break;
		}
	}
	
	switch (g_game_state) {
		case GAME_STATE_SELECT_SOURCE: {
			switch (key) {
				case sk_Sin:
					// TODO
					//g_storages[g_src_sto].data[g_src_index] ^= CARD_FACING_MASK;
				break;

				case sk_Store:
					if (g_storages[DECK].usage > 0) {
						++g_move_count;
						struct Card card;
						cs_take_top_card(&g_storages[DECK], &card);
						card_set_facing(card, CARD_FACING_UP);
						cs_add_card(
							&g_storages[DISCARD],
							card
						);
						g_src_sto = DISCARD;
						g_src_index = g_storages[DISCARD].usage - 1;
					}
					else {
						++g_move_count;
						while (g_storages[DISCARD].usage > 0) {
							struct Card card;
							cs_take_top_card(&g_storages[DISCARD], &card);
							card_set_facing(card, CARD_FACING_DOWN);
							cs_add_card(
								&g_storages[DECK],
								card
							);
						}
						g_src_sto = COLUM;
						g_src_index = g_storages[COLUM].usage - 1;
					}
				break;

				case sk_Window:
					if (g_storages[SCORING + 0].usage > 0)
						set_source_colum(SCORING + 0);
				break;
		
				case sk_Zoom:
					if (g_storages[SCORING + 1].usage > 0)
						set_source_colum(SCORING + 1);
				break;
		
				case sk_Trace:
					if (g_storages[SCORING + 2].usage > 0)
						set_source_colum(SCORING + 2);
				break;
		
				case sk_Graph:
					if (g_storages[SCORING + 3].usage > 0)
						set_source_colum(SCORING + 3);
				break;
		
				case sk_Yequ:
				case sk_0:
					if (g_storages[1].usage > 0)
						set_source_colum(1);
				break;
		
				case sk_1:
					if (g_storages[COLUM + 0].usage > 0)
						set_source_colum(COLUM + 0);
				break;
		
				case sk_2:
					if (g_storages[COLUM + 1].usage > 0)
						set_source_colum(COLUM + 1);
				break;
		
				case sk_3:
					if (g_storages[COLUM + 2].usage > 0)
						set_source_colum(COLUM + 2);
				break;
		
				case sk_4:
					if (g_storages[COLUM + 3].usage > 0)
						set_source_colum(COLUM + 3);
				break;
		
				case sk_5:
					if (g_storages[COLUM + 4].usage > 0)
						set_source_colum(COLUM + 4);
				break;
		
				case sk_6:
					if (g_storages[COLUM + 5].usage > 0)
						set_source_colum(COLUM + 5);
				break;
		
				case sk_7:
					if (g_storages[COLUM + 6].usage > 0)
						set_source_colum(COLUM + 6);
				break;
		
				case sk_Right:
					++g_src_sto;
					while (g_storages[g_src_sto].usage == 0 || g_src_sto == 0) {
						++g_src_sto;
						if (g_src_sto > 12)
							g_src_sto %= 13;
					}
					g_src_index = g_storages[g_src_sto].usage - 1;
				break;
		
				case sk_Left:
					--g_src_sto;
					while (g_storages[g_src_sto].usage == 0 || g_src_sto == 0) {
						if (g_src_sto == 0)
							g_src_sto = 12;
						else
							--g_src_sto;
					}
					g_src_index = g_storages[g_src_sto].usage - 1;
				break;
		
				case sk_Down:
				if (g_src_index < g_storages[g_src_sto].usage - 1 && (COLUM <= g_src_sto && g_src_sto < SCORING)) {
					++g_src_index;
				}
				break;
		
				case sk_Up:
				if (g_src_index > 0 && (COLUM <= g_src_sto && g_src_sto < SCORING)) {
					--g_src_index;
				}
				break;

				case sk_Power:
					for (uint8_t i = SCORING; i < SCORING + 4; ++i) {
						if (is_valid_klondike_g_tar_sto(i)) {
							++g_move_count;
							cs_move_cards(&g_storages[g_src_sto], &g_storages[i], g_src_index, g_storages[i].usage, 0);
							if (g_storages[g_src_sto].usage > 0)
								card_set_facing(g_storages[g_src_sto].data[g_src_index - 1], CARD_FACING_UP);
							g_src_index = g_storages[g_src_sto].usage - 1;
							break;
						}
					}
				break;
		
				case sk_2nd:
				case sk_Enter:
				if (is_valid_klondike_g_src_index(g_src_index)) {
					g_tar_sto = g_src_sto;
					g_game_state = GAME_STATE_SELECT_TARGET;
				}
				break;
			}

		} break;

		case GAME_STATE_SELECT_TARGET: {
			switch (key) {
				case sk_Window:
					g_tar_sto = SCORING + 0;
				break;
		
				case sk_Zoom:
					g_tar_sto = SCORING + 1;
				break;
		
				case sk_Trace:
					g_tar_sto = SCORING + 2;
				break;
		
				case sk_Graph:
					g_tar_sto = SCORING + 3;
				break;
		
				case sk_1:
					g_tar_sto = COLUM + 0;
				break;
		
				case sk_2:
					g_tar_sto = COLUM + 1;
				break;
		
				case sk_3:
					g_tar_sto = COLUM + 2;
				break;
		
				case sk_4:
					g_tar_sto = COLUM + 3;
				break;
		
				case sk_5:
					g_tar_sto = COLUM + 4;
				break;
		
				case sk_6:
					g_tar_sto = COLUM + 5;
				break;
		
				case sk_7:
					g_tar_sto = COLUM + 6;
				break;
		
				case sk_Right:
					do {
						++g_tar_sto;
						if (g_tar_sto > 12)
							g_tar_sto %= 13;
						if (g_tar_sto != 0 && g_tar_sto != 1)
							break;
					} while (true);
				break;
		
				case sk_Left:
					do {
						if (g_tar_sto == 0)
							g_tar_sto = 12;
						else
							--g_tar_sto;
						if (g_tar_sto != 0 && g_tar_sto != 1)
							break;
					} while (true);
				break;

				case sk_Power:
					for (uint8_t i = SCORING; i < SCORING + 4; ++i) {
						if (is_valid_klondike_g_tar_sto(i)) {
							g_tar_sto = i;
							g_valid_target = true;
							break;
						}
					}
				break;

				case sk_2nd:
				case sk_Enter:
					if (g_valid_target) {
						++g_move_count;
						cs_move_cards(&g_storages[g_src_sto], &g_storages[g_tar_sto], g_src_index, g_storages[g_tar_sto].usage, 0);
						if (g_storages[g_src_sto].usage > 0)
							card_set_facing(g_storages[g_src_sto].data[g_src_index - 1], CARD_FACING_UP);
					}
					g_game_state = GAME_STATE_SELECT_SOURCE;
					if (g_tar_sto >= COLUM && g_tar_sto < COLUM + 7)
						g_src_sto = g_tar_sto;
					g_src_index = g_storages[g_src_sto].usage - 1;
					g_tar_sto = 0;
				break;
			}
		} break;

		case GAME_STATE_WON: {
			if (key) {
				g_game_state = GAME_STATE_NULL;
				g_program_state = PROGRAM_STATE_MAIN_MENU;
			}
		} break;
	}
}

// main looping logic
bool step()
{
	uint8_t key = os_GetCSC();
	if (key == sk_Clear) {
		return true;
	}

	switch (g_program_state) {
		case PROGRAM_STATE_MAIN_MENU: {
			if (key) {
				init_klondike();
				g_program_state = PROGRAM_STATE_KLONDIKE_IN_GAME;
				g_game_state = GAME_STATE_SELECT_SOURCE;
				g_game_start_time = clock();
				rand();
			}
		} break;

		case PROGRAM_STATE_KLONDIKE_IN_GAME:
			klondike_step(key);
		break;
	}
	g_valid_target = is_valid_klondike_g_tar_sto(g_tar_sto);
    return false;
}

int drawCard(struct Card card, int x, int y, bool highlight) {
	// facedown
	if (card_facing(card) == CARD_FACING_DOWN) {
		gfx_TransparentSprite_NoClip(bard_backv2, x, y);
	}
	else {
		switch (card.value) {
			case CARD_VALUE_KING:
				gfx_TransparentSprite_NoClip(king, x, y);
			break;

			case CARD_VALUE_QUEEN:
				gfx_TransparentSprite_NoClip(queen, x, y);
			break;

			case CARD_VALUE_JACK:
				gfx_TransparentSprite_NoClip(jack, x, y);
			break;

			default:
			gfx_TransparentSprite_NoClip(blank_cardv2, x, y);
		}

		int glif1_x = x + 2;
		int glif1_y = y + 5;
		int glif2_x = x + CARD_WIDTH - 2 - 8;
		int glif2_y = y + CARD_HEIGHT - 10 - 5;
		uint8_t value = card.value - 1;

		gpfx_monoMaskSprite(
			*(gfx_vbuffer + glif1_y) + glif1_x,
			(10 << 8) + (2 + card_suit(card)),
			data + 10 * value
		);
		gpfx_monoMaskSprite_flipped(
			*(gfx_vbuffer + glif2_y) + glif2_x,
			(10 << 8) + (2 + card_suit(card)),
			data + 10 * value
		);
		unsigned int height_color = (7 << 8) + (2 + card_suit(card));
		void* glif = data + 130 + 7 * card_suit(card);
		uint8_t sum = (value*(value+1)) / 2;
		if (value < 10) {
			for (uint8_t i = 0; i <= value; ++i) {
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
		gfx_SetColor(COLOR_UNKNOWN_SELECTION);
		gfx_Rectangle_NoClip(x - 2, y - 2, CARD_WIDTH + 4, CARD_HEIGHT + 4);
	}
	return 0;
}

void drawShadowText(const char* str, uint8_t x, uint8_t y, uint8_t scale) {
	gfx_SetTextScale(scale, scale);
	gfx_SetTextFGColor(5);
	gfx_PrintStringXY(str, x + scale, y + scale);
	
	gfx_SetTextFGColor(COLOR_WHITE);
	gfx_PrintStringXY(str, x, y);
}

void drawKlondike() {
	gfx_ZeroScreen();
	// playfield
	for (uint8_t colum = COLUM; colum < COLUM + 7; ++colum) {
		uint8_t coverd_card_height = min((165 - CARD_HEIGHT) / (g_storages[colum].usage - 1), 15);
		for (uint8_t card_index = 0; card_index < g_storages[colum].usage; ++card_index) {
			drawCard(
				g_storages[colum].data[card_index],
				(colum - COLUM) * (CARD_WIDTH + 6) + 4, card_index * coverd_card_height + 60,
				colum == g_src_sto && card_index == g_src_index
			);
		}
		if (colum == g_tar_sto) {
			if (g_valid_target)
				gfx_SetColor(COLOR_VALID_SELECTION);
			else
				gfx_SetColor(COLOR_INVALID_SELECTION);
			gfx_Rectangle_NoClip(
				(colum - COLUM) * (CARD_WIDTH + 6) + 2,
				58,
				CARD_WIDTH + 4,
				CARD_HEIGHT + 4 + max((g_storages[colum].usage - 1) * coverd_card_height, 1)
			);
		}
	}

	// draw the deck
	uint8_t num_to_draw = min(3, g_storages[DECK].usage);
	for (uint8_t i = 0; i < num_to_draw; ++i) {
		drawCard(
			g_storages[DECK].data[g_storages[DECK].usage + i - num_to_draw],
			2 + i * 16, 2,
			DECK == g_src_sto && (g_storages[DECK].usage + i - num_to_draw) == g_src_index
		);
	}
	// draw the discard pile
	num_to_draw = min(3, g_storages[DISCARD].usage);
	for (uint8_t i = 0; i < num_to_draw; ++i) {
		drawCard(
			g_storages[DISCARD].data[g_storages[DISCARD].usage + i - num_to_draw],
			80 + i * 16, 2,
			DISCARD == g_src_sto && (g_storages[DISCARD].usage + i - num_to_draw) == g_src_index
		);
	}
	// draw the scoring piles
	for (uint8_t i = SCORING; i < SCORING + 4; ++i) {
		if (g_storages[i].usage > 0) {
			drawCard(
				top_card(i),
				150 + (i - SCORING) * (CARD_WIDTH + 4), 2,
				i == g_src_sto && g_storages[i].usage - 1 == g_src_index
			);
		}
		else {
			gpfx_monoMaskSprite(
			*(gfx_vbuffer + CARD_HEIGHT/2 - 4) + 150 + CARD_WIDTH/2 - 4 + 1 + (i - SCORING) * (CARD_WIDTH + 4),
			(7 << 8) + (5),
			data + 130 + 7 * (i - SCORING)
			);
		}
		if (i == g_tar_sto) {
			if (g_valid_target)
				gfx_SetColor(COLOR_VALID_SELECTION);
			else
				gfx_SetColor(COLOR_INVALID_SELECTION);
			gfx_Rectangle_NoClip(150 - 2 + (i - SCORING) * (CARD_WIDTH + 4), 0, CARD_WIDTH + 4, CARD_HEIGHT + 4);
		}
	}
	gfx_SetTextScale(2, 2);
	gfx_PrintStringXY("Time: ", 5, 224);
	gfx_PrintInt((clock() - g_game_start_time) / 32768, 1);

	gfx_PrintStringXY("Moves: ", 160, 224);
	gfx_PrintInt(g_move_count, 1);
}

void drawMainMenu() {
	
	gfx_ZeroScreen();
	gfx_SetTextScale(4, 4);
	drawShadowText("Solitaire", GFX_LCD_WIDTH/2 - gfx_GetStringWidth("Solitaire")/2, 20, 4);
	drawShadowText("CElection", GFX_LCD_WIDTH/2 - gfx_GetStringWidth("CElection")/2, 60, 4);
	drawShadowText("Press any key to start", 10, 150, 2);

	//drawCard(CARD_SPADES | CARD_A, 30, 180, false);
	//drawCard(CARD_HEARTS | CARD_K, GFX_LCD_WIDTH - 30 - CARD_WIDTH, 180, false);
}

// draw graphics
void draw()
{
	switch (g_program_state) {
		case PROGRAM_STATE_MAIN_MENU:
			drawMainMenu();
		break;

		case PROGRAM_STATE_KLONDIKE_IN_GAME:
			switch (g_game_state) {
				case GAME_STATE_SELECT_SOURCE:
				case GAME_STATE_SELECT_TARGET:
					drawKlondike();
				break;

				case GAME_STATE_WON:
					gfx_SetTextScale(4, 4);
					drawShadowText("You Win!", GFX_LCD_WIDTH/2 - gfx_GetStringWidth("You Win!")/2, 20, 4);
					gfx_SetTextScale(3, 3);
					drawShadowText("Press any key", GFX_LCD_WIDTH/2 - gfx_GetStringWidth("Press any key")/2, 100, 3);
				break;
			}
		break;
	}
}