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
			state_clear(state);
			state->program_state = PROGRAM_STATE_MAIN_MENU;
		} break;

		case PROGRAM_STATE_MAIN_MENU: {
			if (key) {
				if (key == sk_Sin) state_set_is_sin_mode(*state, true);
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
			kd_fx_draw(state);
		break;
	}
	dbg_printf("step + frame took %d\n", (int)((clock() / (CLOCKS_PER_SEC/1000)) - state->time_frame_begin));
}