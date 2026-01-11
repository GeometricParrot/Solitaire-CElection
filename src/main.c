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
	GAME_STATE_AUTOWIN,
	GAME_STATE_WINNING_ANIMATION,
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
#define buffer_position(x, y) (*(gfx_vbuffer + y) + x)

int g_program_state = PROGRAM_STATE_MAIN_MENU;
int g_game_state = GAME_STATE_NULL;
uint8_t g_src_sto = 0;
uint8_t g_src_index = 0;
uint8_t g_tar_sto = 0;
clock_t g_game_start_time = 0;
int g_move_count = 0;
bool g_valid_target = false;
clock_t g_start_of_frame = 0;

#define CARD_STORAGE_NUMBER 13
CardStorage g_storages[CARD_STORAGE_NUMBER];

#define AQ_CAPACITY 1
struct Animation {
	uint8_t storage;
	uint8_t y;
	uint24_t x;
	gfx_sprite_t* behind_sprite;
	struct Card* card;
};

int drawCard(struct Card* card, bool highlight);

void animate(struct Animation* animation) {
	animation->card->source_x = (animation->x + animation->card->source_x) / 2;
	animation->card->source_y = (animation->y + animation->card->source_y) / 2;
}
struct Animation g_animation_queue;

void submit_animation(struct Card* card, uint24_t x, uint8_t y, uint8_t sto_index) {
	if (g_animation_queue.card == NULL) {
		dbg_printf("submit_animation() called with life %d\n", card->life_remaining);
		g_animation_queue.card = card;
		g_animation_queue.x = x;
		g_animation_queue.y = y;
		g_animation_queue.storage = sto_index;
	}
}

void render_and_animate_cards() {
	if (g_animation_queue.card != NULL) {
		dbg_printf("render_and_animate_cards()\n");
		animate(&g_animation_queue);
		gfx_GetSprite_NoClip(g_animation_queue.behind_sprite, g_animation_queue.card->source_x, g_animation_queue.card->source_y);
		drawCard(
			g_animation_queue.card,
			false
		);
		--g_animation_queue.card->life_remaining;
		if (g_animation_queue.card->life_remaining == 0) {
			dbg_printf("animate() redraw storage %d\n", g_animation_queue.storage);
			g_storages[g_animation_queue.storage].requires_redraw = true;
			g_animation_queue.card = NULL;
		}
	}
}

bool step();
void draw();
void init_klondike();

int main(void)
{
	srand(time(NULL));

	gfx_Begin();
	gfx_SetPalette(global_palette, sizeof_global_palette, 0);
	gfx_SetTransparentColor(COLOR_TRANSPARENT);
    gfx_SetDrawBuffer();
	//gfx_SetDrawScreen();


    while (!step())
    {
        draw();

		//gfx_BlitBuffer();
        //gfx_SwapDraw();
	}

    gfx_End();

    return 0;
}

void std_deck_deal() {
	for (uint8_t i = 0; i < 52; ++i) {
		struct Card card = {(i%CARD_STORAGE_NUMBER) + 1, i/CARD_STORAGE_NUMBER, 0, 0, 0};
		cs_insert_to_top_card(&g_storages[DECK], card);
	}
	cs_shuffle(&g_storages[DECK]);
	for (uint8_t i = 0; i < 7; ++i) {
		for (uint8_t j = i; j < 7; ++j) {
			struct Card card;
			card.life_remaining = 2;
			cs_take_top_card(&g_storages[DECK], &card);
			if (i == j) {
				card_set_facing(card, CARD_FACING_UP);
			}
			else {
				card_set_facing(card, CARD_FACING_DOWN);
			}
			cs_insert_to_top_card(&g_storages[COLUM + j], card);
		}
	}

	g_src_sto = COLUM;
	g_src_index = g_storages[g_src_sto].usage - 1;
}

void init_klondike() {
	dbg_printf("init_klondike()\n");
	gfx_SetDrawBuffer();
	gfx_ZeroScreen();

	for (uint8_t i = 0; i < CARD_STORAGE_NUMBER; ++i) {
		g_storages[i].requires_redraw = true;
	}

	if (g_animation_queue.behind_sprite == NULL) {
		g_animation_queue.behind_sprite = gfx_MallocSprite(CARD_WIDTH, CARD_HEIGHT);
	}
	g_animation_queue.card = NULL;
}

void set_source_colum(uint8_t colum) {
	g_storages[g_src_sto].requires_redraw = true;
	g_storages[colum].requires_redraw = true;
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
					card_suit(top_card(storage)) != card_suit(g_storages[g_src_sto].data[g_src_index])
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

void set_target_colum(uint8_t colum) {
	g_storages[g_tar_sto].requires_redraw = true;
	g_storages[colum].requires_redraw = true;
	g_tar_sto = colum;
	g_valid_target = is_valid_klondike_g_tar_sto(colum);
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
	switch (g_game_state) {
		case GAME_STATE_SELECT_SOURCE: {
			switch (key) {
				case sk_Math:
					if (klondike_is_auto_winnable())
						g_game_state = GAME_STATE_AUTOWIN;
				break;

				case sk_Sin:
					// TODO
					//g_storages[g_src_sto].data[g_src_index] ^= CARD_FACING_MASK;
				break;

				case sk_Store:
					if (g_storages[DECK].usage > 0) {
						g_storages[DECK].requires_redraw = true;
						g_storages[DISCARD].requires_redraw = true;
						++g_move_count;
						cs_debug_print(&g_storages[DISCARD]);
						cs_move_top_card_to_top(&g_storages[DECK], &g_storages[DISCARD]);
						cs_debug_print(&g_storages[DISCARD]);
						card_set_facing(top_card(DISCARD), CARD_FACING_UP);
						set_source_colum(DISCARD);
						g_src_index = g_storages[DISCARD].usage - 1;
					}
					else {
						g_storages[DECK].requires_redraw = true;
						g_storages[DISCARD].requires_redraw = true;
						++g_move_count;
						while (g_storages[DISCARD].usage > 0) {
							cs_move_top_card_to_top(&g_storages[DISCARD], &g_storages[DECK]);
							card_set_facing(top_card(DECK), CARD_FACING_DOWN);
						}
						set_source_colum(COLUM);
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
						set_source_colum(DISCARD);
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
					set_source_colum(g_src_sto + 1);
					if (g_src_sto >= CARD_STORAGE_NUMBER)
						set_source_colum(g_src_sto - CARD_STORAGE_NUMBER);
					while (g_storages[g_src_sto].usage == 0 || g_src_sto == 0) {
						set_source_colum(g_src_sto + 1);
						if (g_src_sto >= CARD_STORAGE_NUMBER)
							set_source_colum(g_src_sto - CARD_STORAGE_NUMBER);
					}
					g_src_index = g_storages[g_src_sto].usage - 1;
				break;
		
				case sk_Left:
					set_source_colum(g_src_sto - 1);
					while (g_storages[g_src_sto].usage == 0 || g_src_sto == 0) {
						if (g_src_sto == 0)
							set_source_colum(CARD_STORAGE_NUMBER - 1);
						else
							set_source_colum(g_src_sto - 1);
					}
					g_src_index = g_storages[g_src_sto].usage - 1;
				break;
		
				case sk_Down:
				if (g_src_index < g_storages[g_src_sto].usage - 1 && (COLUM <= g_src_sto && g_src_sto < SCORING)) {
					g_storages[g_src_sto].requires_redraw = true;
					++g_src_index;
				}
				break;
		
				case sk_Up:
				if (g_src_index > 0 && (COLUM <= g_src_sto && g_src_sto < SCORING)) {
					g_storages[g_src_sto].requires_redraw = true;
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
					set_target_colum(g_src_sto);
					g_game_state = GAME_STATE_SELECT_TARGET;
				}
				break;
			}

		} break;

		case GAME_STATE_SELECT_TARGET: {
			switch (key) {
				case sk_Math:
					if (klondike_is_auto_winnable())
						g_game_state = GAME_STATE_AUTOWIN;
				break;

				case sk_Del:
				case sk_Alpha:
					g_game_state = GAME_STATE_SELECT_SOURCE;
					set_target_colum(0);
					//while (!is_valid_klondike_g_src_index(g_src_index)) {
					//	set_source_colum(g_src_sto + 1);
					//	if (g_src_sto >= CARD_STORAGE_NUMBER)
					//		set_source_colum(g_src_sto - CARD_STORAGE_NUMBER);
					//	g_src_index = g_storages[g_src_sto].usage - 1;
					//}
				break;
				case sk_Window:
					set_target_colum(SCORING + 0);
				break;
		
				case sk_Zoom:
					set_target_colum(SCORING + 1);
				break;
		
				case sk_Trace:
					set_target_colum(SCORING + 2);
				break;
		
				case sk_Graph:
					set_target_colum(SCORING + 3);
				break;
		
				case sk_1:
					set_target_colum(COLUM + 0);
				break;
		
				case sk_2:
					set_target_colum(COLUM + 1);
				break;
		
				case sk_3:
					set_target_colum(COLUM + 2);
				break;
		
				case sk_4:
					set_target_colum(COLUM + 3);
				break;
		
				case sk_5:
					set_target_colum(COLUM + 4);
				break;
		
				case sk_6:
					set_target_colum(COLUM + 5);
				break;
		
				case sk_7:
					set_target_colum(COLUM + 6);
				break;
		
				case sk_Right:
					do {
						set_target_colum(g_tar_sto + 1);
						if (g_tar_sto >= CARD_STORAGE_NUMBER)
							set_target_colum(g_tar_sto - CARD_STORAGE_NUMBER);
						if (g_tar_sto != 0 && g_tar_sto != 1)
							break;
					} while (true);
				break;
		
				case sk_Left:
					do {
						if (g_tar_sto == 0)
							set_target_colum(CARD_STORAGE_NUMBER - 1);
						else
							set_target_colum(g_tar_sto - 1);
						if (g_tar_sto != 0 && g_tar_sto != 1)
							break;
					} while (true);
				break;

				case sk_Power:
					for (uint8_t i = SCORING; i < SCORING + 4; ++i) {
						if (is_valid_klondike_g_tar_sto(i)) {
							set_target_colum(i);
							break;
						}
					}
				break;

				case sk_2nd:
				case sk_Enter:
					if (g_valid_target) {
						++g_move_count;
						cs_move_cards(
							&g_storages[g_src_sto], &g_storages[g_tar_sto],
							g_src_index, g_storages[g_tar_sto].usage, 0
						);
						if (g_storages[g_src_sto].usage > 0)
							card_set_facing(g_storages[g_src_sto].data[g_src_index - 1], CARD_FACING_UP);
					}
					g_game_state = GAME_STATE_SELECT_SOURCE;
					if (g_tar_sto >= COLUM && g_tar_sto < COLUM + 7)
						set_source_colum(g_tar_sto);
					g_src_index = g_storages[g_src_sto].usage - 1;
					set_target_colum(DECK);
				break;
			}
		} break;

		case GAME_STATE_AUTOWIN: {
			if (klondike_is_won() && g_animation_queue.card == NULL && false) {
				g_game_state = GAME_STATE_WON;
				dbg_printf("set g_game_state = GAME_STATE_WON\n");
			} else {
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
								dbg_printf("Moving card for autowin\n");
								cs_move_top_card_to_top(&g_storages[source], &g_storages[target]);
								return;
							}
						}
					}
				}
				dbg_printf("Something went wrong with the auto win\n");
			}
		} break;

		case GAME_STATE_WINNING_ANIMATION: {
			//uint8_t temp = rand() % 4;
			//submit_animation(&top_card(temp), rand() % (320 - CARD_WIDTH), rand() % (240 - CARD_HEIGHT), temp);
			if (key == sk_2nd) {
				g_game_state = GAME_STATE_WON;
			}
		} break;

		case GAME_STATE_WON: {
			switch (key) {
				case sk_2nd:
					for (uint8_t i = 0; i < CARD_STORAGE_NUMBER; ++i) {
						cs_zero(&g_storages[i]);
					}
					g_game_state = GAME_STATE_NULL;
					g_program_state = PROGRAM_STATE_MAIN_MENU;
				break;
			}
		} break;
	}
}

// main looping logic
bool step()
{
	g_start_of_frame = clock() / (CLOCKS_PER_SEC/1000);
	uint8_t key = os_GetCSC();
	if (key == sk_Clear) {
		return true;
	}
	dbg_printf("program state: %d, game state: %d\n", g_program_state, g_game_state);

	switch (g_program_state) {
		case PROGRAM_STATE_MAIN_MENU: {
			if (key == sk_Sin) {
				init_klondike();
				for (uint8_t colum = COLUM; colum < COLUM + 4; ++colum) {
					for (uint8_t value = CARD_VALUE_KING; value >= CARD_VALUE_ACE; --value) {
						struct Card card;
						card.value = value;
						card_set_facing(card, CARD_FACING_UP);
						card_set_suit(card, (colum - 2 + (2 * value)) & 0b11);
						cs_insert_to_top_card(&g_storages[colum], card);
					}
				}
				g_program_state = PROGRAM_STATE_KLONDIKE_IN_GAME;
				g_game_state = GAME_STATE_SELECT_SOURCE;
				g_game_start_time = clock();
			} else if (key) {
				init_klondike();
				std_deck_deal();
				g_program_state = PROGRAM_STATE_KLONDIKE_IN_GAME;
				g_game_state = GAME_STATE_SELECT_SOURCE;
				g_game_start_time = clock();
			}
		} break;

		case PROGRAM_STATE_KLONDIKE_IN_GAME:
			klondike_step(key);
		break;
	}
    return false;
}

int drawCard(struct Card* card, bool highlight) {
	// facedown
	if (card_facing((*card)) == CARD_FACING_DOWN) {
		gfx_TransparentSprite_NoClip(bard_backv2, card->source_x, card->source_y);
	}
	else {
		switch (card->value) {
			case CARD_VALUE_KING:
				gfx_TransparentSprite_NoClip(king, card->source_x, card->source_y);
			break;

			case CARD_VALUE_QUEEN:
				gfx_TransparentSprite_NoClip(queen, card->source_x, card->source_y);
			break;

			case CARD_VALUE_JACK:
				gfx_TransparentSprite_NoClip(jack, card->source_x, card->source_y);
			break;

			default:
			gfx_TransparentSprite_NoClip(blank_cardv2, card->source_x, card->source_y);
		}

		uint8_t* const glif1 = buffer_position(card->source_x + 2, card->source_y + 5);
		uint8_t* const glif2 = buffer_position(card->source_x + CARD_WIDTH - 2 - 8, card->source_y + CARD_HEIGHT - 10 - 5);
		uint8_t const value = card->value - 1;

		// number glifs
		gpfx_monoMaskSprite(
			glif1,
			(10 << 8) + (2 + card_suit((*card))),
			data + 10 * value
		);
		gpfx_monoMaskSprite_flipped(
			glif2,
			(10 << 8) + (2 + card_suit((*card))),
			data + 10 * value
		);
		uint24_t height_color = (7 << 8) + (2 + card_suit((*card)));
		const uint8_t* const glif = data + 130 + 7 * card_suit((*card));
		if (value < 10) {
			uint8_t const sum = (value*(value+1)) / 2;
			for (uint8_t i = 0; i <= value; ++i) {
				gpfx_monoMaskSprite(
					buffer_position(card->source_x + glif_locations_x[sum + i], card->source_y + glif_locations_y[sum + i]),
					height_color,
					glif
				);
			}
		}
		else {
			gpfx_monoMaskSprite(
				buffer_position(card->source_x + CARD_WIDTH - 3 - GLIF_SMALL_WIDTH, card->source_y + 5),
				height_color,
				glif
			);
			gpfx_monoMaskSprite_flipped(
				buffer_position(card->source_x + 1, card->source_y + CARD_HEIGHT - GLIF_SMALL_HEIGHT - 5),
				height_color,
				glif
			);
		}
	}
	// box to highlight selected card
	if (highlight) {
		gfx_SetColor(COLOR_UNKNOWN_SELECTION);
		gfx_Rectangle_NoClip(card->source_x - 2, card->source_y - 2, CARD_WIDTH + 4, CARD_HEIGHT + 4);
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

void klondike_clear_screen() {
	// clear the screen areas that need redrawn
	if (g_animation_queue.card != NULL && g_animation_queue.card->life_remaining > 0) {
		dbg_printf("clearing animated card beck with life %d\n", g_animation_queue.card->life_remaining);
		gfx_Sprite_NoClip(
			g_animation_queue.behind_sprite,
			g_animation_queue.card->source_x,
			g_animation_queue.card->source_y
		);
	}
	gfx_SetColor(0);
	if (g_storages[DECK].requires_redraw) { // deck
		dbg_printf("Clearing Deck\n");
		gfx_FillRectangle_NoClip(0, 0, CARD_WIDTH + 2*12 + 4, CARD_HEIGHT + 4);
	}
	if (g_storages[DISCARD].requires_redraw) { // discard
		dbg_printf("Clearing Discard\n");
		gfx_FillRectangle_NoClip(80 - 2, 0, CARD_WIDTH + 2*12 + 4, CARD_HEIGHT + 4);
	}
	for (uint8_t i = SCORING; i < SCORING + 4; ++i) { // scoring
		if (g_storages[i].requires_redraw) {
			dbg_printf("Clearing Scoring %d\n", i);
			gfx_FillRectangle_NoClip(150 + (i - SCORING) * (CARD_WIDTH + 4) - 2, 0, CARD_WIDTH + 4, CARD_HEIGHT + 4);
		}
	}
	for (uint8_t colum = COLUM; colum < COLUM + 7; ++colum) { // colum
		if (g_storages[colum].requires_redraw ) {
			dbg_printf("Clearing Colum %d\n", colum);
			gfx_FillRectangle_NoClip(
				(colum - COLUM) * (CARD_WIDTH + 6),
				CARD_HEIGHT + 4 + 1,
				CARD_WIDTH + 4,
				160
			);
		}
	}
	gfx_FillRectangle_NoClip(5, 224, 300, 16);
}

void drawKlondike() {
	klondike_clear_screen();

	// draw the deck
	if (g_storages[DECK].requires_redraw) {
		dbg_printf("Drawing Deck\n");
		g_storages[DECK].requires_redraw = false;
		uint8_t num_to_draw = min(3, g_storages[DECK].usage);
		for (uint8_t i = 0; i < num_to_draw; ++i) {
			struct Card* card = &g_storages[DECK].data[g_storages[DECK].usage + i - num_to_draw];
			card->source_x = 2 + i * 12;
			card->source_y = 2;
			drawCard(
				card,
				DECK == g_src_sto && (g_storages[DECK].usage + i - num_to_draw) == g_src_index
			);
		}
	}

	// draw the discard pile
	if (g_storages[DISCARD].requires_redraw) {
		dbg_printf("Drawing Discard\n");
		g_storages[DISCARD].requires_redraw = false;
		uint8_t num_to_draw = min(3, g_storages[DISCARD].usage);
		for (uint8_t i = 0; i < num_to_draw; ++i) {
			struct Card* card = &g_storages[DISCARD].data[g_storages[DISCARD].usage + i - num_to_draw];
			card->source_x = 80 + i * 12;
			card->source_y = 2;
			drawCard(
				card,
				DISCARD == g_src_sto && (g_storages[DISCARD].usage + i - num_to_draw) == g_src_index
			);
		}
		if (g_tar_sto == DISCARD) {
			gfx_SetColor((g_valid_target) ? COLOR_VALID_SELECTION : COLOR_INVALID_SELECTION);
			gfx_Rectangle_NoClip(
				78, 0,
				CARD_WIDTH + 4 + num_to_draw * 12 - 12,
				CARD_HEIGHT + 4
			);
		}
	}

	// draw the scoring piles
	for (uint8_t i = SCORING; i < SCORING + 4; ++i) {
		if (g_storages[i].requires_redraw) {
			dbg_printf("Drawing scoring %d\n", i);
			g_storages[i].requires_redraw = false;
			if (g_storages[i].usage > 0) {
				const uint24_t x = 150 + (i - SCORING) * (CARD_WIDTH + 4);
				const uint8_t y = 2;
				struct Card* card = &top_card(i);
				if (card->life_remaining != 0) {
					submit_animation(card, x, y, i);
					if (g_storages[i].usage > 1) {
						card = &g_storages[i].data[g_storages[i].usage - 2];
						card_dbg_print(*card);
						card->source_x = x;
						card->source_y = y;
						drawCard(
							card,
							false
						);
					}
				} else {
					card->source_x = x;
					card->source_y = y;
					drawCard(
						card,
						i == g_src_sto && g_storages[i].usage - 1 == g_src_index
					);
				}
			}
			else {
				gpfx_monoMaskSprite(
				*(gfx_vbuffer + CARD_HEIGHT/2 - 4) + 150 + CARD_WIDTH/2 - 4 + 1 + (i - SCORING) * (CARD_WIDTH + 4),
				(7 << 8) + (5),
				data + 130 + 7 * (i - SCORING)
				);
			}
			if (i == g_tar_sto) {
				gfx_SetColor((g_valid_target) ? COLOR_VALID_SELECTION : COLOR_INVALID_SELECTION);
				gfx_Rectangle_NoClip(150 - 2 + (i - SCORING) * (CARD_WIDTH + 4), 0, CARD_WIDTH + 4, CARD_HEIGHT + 4);
			}
		}
	}

	// playfield
	for (uint8_t colum = COLUM; colum < COLUM + 7; ++colum) {
		if (g_storages[colum].requires_redraw ) {
			dbg_printf("Drawing Colum %d\n", colum);
			g_storages[colum].requires_redraw = false;
			uint8_t coverd_card_height = min((160 - 4 - CARD_HEIGHT) / (g_storages[colum].usage - 1), 15);
			for (uint8_t card_index = 0; card_index < g_storages[colum].usage; ++card_index) {
				struct Card* card = &g_storages[colum].data[card_index];
				uint24_t x = (colum - COLUM) * (CARD_WIDTH + 6) + 2;
				uint8_t y = card_index * coverd_card_height + CARD_HEIGHT + 4 + 1 + 2;
				if (card->life_remaining != 0) {
					submit_animation(
						card,
						x,
						y,
						colum
					);
				} else {
					card->source_x = x;
					card->source_y = y;
					drawCard(
						card,
						colum == g_src_sto && card_index == g_src_index
					);
				}
			}
			if (colum == g_tar_sto) {
				gfx_SetColor((g_valid_target) ? COLOR_VALID_SELECTION : COLOR_INVALID_SELECTION);
				gfx_Rectangle_NoClip(
					(colum - COLUM) * (CARD_WIDTH + 6),
					CARD_HEIGHT + 4 + 1,
					CARD_WIDTH + 4,
					CARD_HEIGHT + 4 + (max(g_storages[colum].usage, 1) - 1) * coverd_card_height
				);
			}
		}
	}

	gfx_SetTextScale(2, 2);
	gfx_PrintStringXY("Time: ", 5, 224);
	gfx_PrintInt((clock() - g_game_start_time) / CLOCKS_PER_SEC, 1);
	gfx_PrintStringXY("Moves: ", 160, 224);
	gfx_PrintInt(g_move_count, 1);

	render_and_animate_cards();

	gfx_BlitBuffer();
}

void klondike_draw_game_state_won() {
	gfx_ZeroScreen();
	gfx_SetTextScale(4, 4);
	drawShadowText("You Win!", GFX_LCD_WIDTH/2 - gfx_GetStringWidth("You Win!")/2, 20, 4);
	gfx_SetTextScale(3, 3);
	drawShadowText("Press any key", GFX_LCD_WIDTH/2 - gfx_GetStringWidth("Press any key")/2, 100, 3);
	gfx_BlitBuffer();
}

void klondike_draw_game_state_autowin() {
	klondike_clear_screen();
	// draw the scoring piles
	for (uint8_t i = SCORING; i < SCORING + 4; ++i) {
		if (g_storages[i].usage > 0 && g_storages[i].requires_redraw) {
			dbg_printf("Drawing Scoring %d\n", i);
			g_storages[i].requires_redraw = false;
			for (uint8_t j = 0; j < g_storages[i].usage; ++j) {
				struct Card* card = &top_card(i);
				const uint24_t x = 150 + (i - SCORING) * (CARD_WIDTH + 4);
				const uint8_t y = 2;
				if (card->life_remaining != 0) {
					submit_animation(card, x, y, i);
				} else {
					card->source_x = x;
					card->source_y = y;
					drawCard(
						card,
						false
					);
				}
			}
		}
	}
	for (uint8_t colum = COLUM; colum < COLUM + 7; ++colum) {
		if (g_storages[colum].requires_redraw ) {
			dbg_printf("Drawing Colum %d\n", colum);
			g_storages[colum].requires_redraw = false;
			uint8_t coverd_card_height = min((160 - 4 - CARD_HEIGHT) / (g_storages[colum].usage - 1), 15);
			for (uint8_t card_index = 0; card_index < g_storages[colum].usage; ++card_index) {
				struct Card* card = &g_storages[colum].data[card_index];
				uint24_t x = (colum - COLUM) * (CARD_WIDTH + 6) + 2;
				uint8_t y = card_index * coverd_card_height + CARD_HEIGHT + 4 + 1 + 2;
				if (card->life_remaining != 0) {
					submit_animation(
						card,
						x,
						y,
						colum
					);
				} else {
					card->source_x = x;
					card->source_y = y;
					drawCard(
						card,
						colum == g_src_sto && card_index == g_src_index
					);
				}
			}
		}
	}
	render_and_animate_cards();
	gfx_BlitBuffer();
}

void drawMainMenu() {
	
	gfx_ZeroScreen();
	gfx_SetTextScale(4, 4);
	drawShadowText("Solitaire", GFX_LCD_WIDTH/2 - gfx_GetStringWidth("Solitaire")/2, 20, 4);
	drawShadowText("CElection", GFX_LCD_WIDTH/2 - gfx_GetStringWidth("CElection")/2, 60, 4);
	drawShadowText("Press any key to start", 10, 150, 2);

	//drawCard(CARD_SPADES | CARD_A, 30, 180, false);
	//drawCard(CARD_HEARTS | CARD_K, GFX_LCD_WIDTH - 30 - CARD_WIDTH, 180, false);
	gfx_SwapDraw();
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
					klondike_draw_game_state_won();
				break;
				
				case GAME_STATE_AUTOWIN:
					klondike_draw_game_state_autowin();
				break;
			}
		break;
	}
	dbg_printf("step + frame took %d\n", (int)((clock() / (CLOCKS_PER_SEC/1000)) - g_start_of_frame));
}