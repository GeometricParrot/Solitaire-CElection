#include <ti/getcsc.h>
#include <graphx.h>
#include <keypadc.h>
#include <time.h>
#include <debug.h>
#include <string.h>

#include "gfx/gfx.h"
#include "gpfx.h"
#include "bit_sprites.h"
#include "card_storage.h"
#include "card.h"
#include "animation.h"
#include "global_state.h"
#include "klondike.h"

bool step(struct State* state);
void draw(struct State* state);

int main(void)
{
	srand(time(NULL));

	gfx_Begin();
	gfx_SetPalette(global_palette, sizeof_global_palette, 0);
	gfx_SetTransparentColor(COLOR_TRANSPARENT);
    gfx_SetDrawBuffer();

	static struct State state;

    while (!step(&state))
    {
        draw(&state);
	}

    gfx_End();

    return 0;
}

// main looping logic
bool step(struct State* state)
{
	state->time_frame_begin = clock() / (CLOCKS_PER_SEC/1000);
	uint8_t key = os_GetCSC();
	if (key == sk_Clear) {
		return true;
	}
	dbg_printf("program state: %d, game state: %d\n", state->program_state, state->game_state);

	switch (state->program_state) {
		case PROGRAM_STATE_NULL: {
			state->flags = 0;
			state->program_state = PROGRAM_STATE_MAIN_MENU;
			state->game_state = GAME_STATE_NULL;
			state->selection_source = 0;
			state->selection_source_index = 0;
			state->selection_target = 0;
			state->selection_target_index = 0;
			state->game_move_count = 0;
			state->time_game_begin = 0;
			state->time_frame_begin = 0;
			memset(state->animation_queue, 0, AQ_CAPACITY * sizeof(struct Animation));
			for (CardStorage* p = state->storages;
				p < state->storages + sizeof(CardStorage) * CARD_STORAGE_NUMBER;
				p += sizeof(CardStorage)
			) {
				cs_zero(p);
			}
			memset(state->storages, 0, CARD_STORAGE_NUMBER * sizeof(CardStorage));
		} break;

		case PROGRAM_STATE_MAIN_MENU: {
			if (key == sk_Sin) {
				kd_init(state);
				//for (uint8_t colum = COLUM; colum < COLUM + 4; ++colum) {
				//	for (uint8_t value = CARD_VALUE_KING; value >= CARD_VALUE_ACE; --value) {
				//		struct Card card;
				//		card.value = value;
				//		card_set_facing(card, CARD_FACING_UP);
				//		card_set_suit(card, (colum - 2 + (2 * value)) & 0b11);
				//		cs_insert_to_top_card(&state->storages[colum], card);
				//	}
				//}
				state->program_state = PROGRAM_STATE_KLONDIKE_IN_GAME;
				state->game_state = GAME_STATE_SELECT_SOURCE;
				state->time_game_begin = clock();
			} else if (key) {
				kd_init(state);
				state->program_state = PROGRAM_STATE_KLONDIKE_IN_GAME;
				state->game_state = GAME_STATE_SELECT_SOURCE;
				state->time_game_begin = clock();
			}
		} break;

		case PROGRAM_STATE_KLONDIKE_IN_GAME:
			kd_step(state, key);
		break;
	}
    return false;
}


void drawMainMenu() {
	
	gfx_ZeroScreen();
	gfx_SetTextScale(4, 4);
	gpfx_drawShadowText("Solitaire", GFX_LCD_WIDTH/2 - gfx_GetStringWidth("Solitaire")/2, 20, 4);
	gpfx_drawShadowText("CElection", GFX_LCD_WIDTH/2 - gfx_GetStringWidth("CElection")/2, 60, 4);
	gpfx_drawShadowText("Press any key to start", 10, 150, 2);

	gfx_SwapDraw();
}

// draw graphics
void draw(struct State* state)
{
	switch (state->program_state) {
		case PROGRAM_STATE_NULL:
		break;
		case PROGRAM_STATE_MAIN_MENU:
			drawMainMenu();
		break;

		case PROGRAM_STATE_KLONDIKE_IN_GAME:
			switch (state->game_state) {
				case GAME_STATE_SELECT_SOURCE:
				case GAME_STATE_SELECT_TARGET:
					kd_fx_draw(state);
				break;

				case GAME_STATE_WON:
				break;
				
				case GAME_STATE_AUTOWIN:
				break;
				case GAME_STATE_NULL:
				break;
				case GAME_STATE_WINNING_ANIMATION:
				break;
			}
		break;
	}
	dbg_printf("step + frame took %d\n", (int)((clock() / (CLOCKS_PER_SEC/1000)) - state->time_frame_begin));
}