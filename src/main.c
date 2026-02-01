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
	dbg_printf("program state: %d, game state: %d\n", state->program_state, state->game_state);

	if ((key == sk_Clear || key == sk_Del) && state->program_state != PROGRAM_STATE_POPUP_MENU) {
		state->popup_previous_state = state->program_state;
		state->program_state = PROGRAM_STATE_POPUP_MENU;
		return false;
		//return true;
	}

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

		case PROGRAM_STATE_POPUP_MENU: {
			dbg_printf("popup\n");
			switch (key) {
				case sk_Del:
				case sk_Clear:
					return true;
				break;

				case sk_Enter:
				case sk_2nd:
				switch (state->menu_selection) {
					case 0:
					state->program_state = state->popup_previous_state;
					state->flags |= 0b100;
					break;

					case 1:
					state->program_state = PROGRAM_STATE_MAIN_MENU;
					state->game_state = GAME_STATE_NULL;
					state_clear(state);
					state->flags |= 0b100;
					break;

					case 2:
					return true; 
					break;
				}
				break;

				case sk_Up:
				if (state->menu_selection == 0) {
					state->menu_selection = 2;
				} else {
					--state->menu_selection;
				}
				break;

				case sk_Down:
				if (state->menu_selection == 2) {
					state->menu_selection = 0;
				} else {
					++state->menu_selection;
				}
				break;
			}
		} break;
	}
    return false;
}


void drawMainMenu() {
	
	gfx_ZeroScreen();
	gfx_SetTextScale(4, 4);
	gpfx_drawShadowText("Solitaire", GFX_LCD_WIDTH/2 - gfx_GetStringWidth("Solitaire")/2, 20, 4);
	gpfx_drawShadowText("CElection", GFX_LCD_WIDTH/2 - gfx_GetStringWidth("CElection")/2, 60, 4);
	gpfx_drawShadowText("Press any key to start", 10, 150, 2);

	gfx_SetTextScale(1, 1);
	gpfx_drawShadowText("super-duper alpha v0.1.0", GFX_LCD_WIDTH/2 - gfx_GetStringWidth("super-duper alpha v0.1.0")/2, 200, 1);

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

		case PROGRAM_STATE_POPUP_MENU:
			gfx_SetColor(10);
			gfx_FillRectangle_NoClip(20, 20, GFX_LCD_WIDTH - 40, GFX_LCD_HEIGHT - 40);
			gpfx_drawShadowText("Resume", 40, 40, 2);
			gpfx_drawShadowText("Main menu", 40, 70, 2);
			gpfx_drawShadowText("Quit", 40, 100, 2);
			gfx_SetColor(COLOR_WHITE);
			gfx_Rectangle_NoClip(30, 35 + (state->menu_selection * 30), GFX_LCD_WIDTH - 60, 25);
			gfx_BlitBuffer();
		break;
	}
	dbg_printf("step + frame took %d\n", (int)((clock() / (CLOCKS_PER_SEC/1000)) - state->time_frame_begin));
}