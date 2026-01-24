#include "klondike.h"
#include "global_state.h"
#include <debug.h>
#include "bit_sprites.h"

void kd_init(struct State* state) {
	dbg_printf("init_klondike()\n");
	gfx_SetDrawBuffer();
	gfx_ZeroScreen();

	aq_init(state->animation_queue);
	if (state_is_sin_mode(*state)) {
		dbg_printf("Starting game in sinner mode.\n");
		for (uint8_t colum = COLUM; colum < COLUM + 4; ++colum) {
			for (uint8_t value = CARD_VALUE_KING; value >= CARD_VALUE_ACE; --value) {
				struct Card card;
				card.target_x = 0;
				card.target_y = 0;
				card.life_remaining = 0;
				card.value = value;
				card_set_facing(card, CARD_FACING_UP);
				card_set_suit(card, (colum - 2 + (2 * value)) & 0b11);
				cs_insert_to_top_card(&state->storages[colum], card);
			}
		}
	} else {
		dbg_printf("Starting normal game.\n");
		state_storage_fill_with_cards(state, DECK);
		cs_shuffle(&state->storages[DECK]);
		for (uint8_t i = 0; i < 7; ++i) {
			for (uint8_t j = i; j < 7; ++j) {
				struct Card card;
				cs_take_top_card(&state->storages[DECK], &card);
				if (i == j) {
					card_set_facing(card, CARD_FACING_UP);
				}
				else {
					card_set_facing(card, CARD_FACING_DOWN);
				}
				cs_insert_to_top_card(&state->storages[COLUM + j], card);
			}
	}
	}
	state_set_source_colum(state, COLUM);
	//state_update_source_index_to_last(state);
}

bool kd_is_autowinnable(struct State* state) {
	if (state->storages[DECK].usage != 0)
	return false;
	if (state->storages[DISCARD].usage > 1)
	return false;
	for (uint8_t colum = COLUM; colum < COLUM + 7; ++colum) {
		for (uint8_t card_index = 0; card_index < state->storages[colum].usage; ++ card_index) {
			if (card_facing(state->storages[colum].data[card_index]) == CARD_FACING_DOWN) {
				return false;
			}
		}
	}
	return true;
}

bool kd_is_won(struct State* state) {
	if (state->storages[DECK].usage != 0)
		return false;
	if (state->storages[DISCARD].usage != 0)
		return false;
	for (uint8_t colum = COLUM; colum < COLUM + 7; ++colum) {
		if (state->storages[colum].usage != 0)
			return false;
	}
	for (uint8_t scoring = SCORING; scoring < SCORING + 4; ++scoring) {
		if (state_storage_top_card(*state, scoring).value != CARD_VALUE_KING && card_suit(state_storage_top_card(*state, scoring)) == scoring - SCORING) {
			return false;
		}
	}
	return true;
}

void kd_step(struct State* state, uint8_t key) {
	switch (state->game_state) {
		case GAME_STATE_SELECT_SOURCE: {
			switch (key) {
				case sk_Math:
					if (kd_is_autowinnable(state))
						state->game_state = GAME_STATE_AUTOWIN;
				break;

				case sk_Sin:
					state->flags ^= 0b00000010;
					dbg_printf("Set sin mode to %d\n", (state->flags & 0b10) >> 1);
				break;

				case sk_Store:
					if (state->storages[DECK].usage > 0) {
						++state->game_move_count;
						if (state->storages[DISCARD].usage >= 1) {
							state->storages[DISCARD].data[state->storages[DISCARD].usage - 1].life_remaining = 4;
						}
						if (state->storages[DISCARD].usage >= 2) {
							state->storages[DISCARD].data[state->storages[DISCARD].usage - 2].life_remaining = 4;
						}
						cs_move_top_card_to_top(&state->storages[DECK], &state->storages[DISCARD], 6);
						card_set_facing(state_storage_top_card(*state, DISCARD), CARD_FACING_UP);
						state_set_source_colum(state, DISCARD);
						state->selection_source_index = state->storages[DISCARD].usage - 1;
						if (state->storages[DECK].usage >= 1) {
							state->storages[DECK].data[state->storages[DECK].usage - 1].life_remaining = 6;
						}
						if (state->storages[DECK].usage >= 2) {
							state->storages[DECK].data[state->storages[DECK].usage - 2].life_remaining = 6;
						}
					}
					else {
						++state->game_move_count;
						while (state->storages[DISCARD].usage > 0) {
							cs_move_top_card_to_top(&state->storages[DISCARD], &state->storages[DECK], 0);
							card_set_facing(state_storage_top_card(*state, DECK), CARD_FACING_DOWN);
							state_storage_top_card(*state, DECK).target_x = 2;
						}
						state_set_source_colum(state, COLUM);
						state->selection_source_index = state->storages[COLUM].usage - 1;
						if (state->storages[DECK].usage >= 1) {
							state->storages[DECK].data[state->storages[DECK].usage - 1].life_remaining = 6;
						}
						if (state->storages[DECK].usage >= 2) {
							state->storages[DECK].data[state->storages[DECK].usage - 2].life_remaining = 6;
						}
					}
				break;

				case sk_Window:
					if (state->storages[SCORING + 0].usage > 0)
						state_set_source_colum(state, SCORING + 0);
				break;
		
				case sk_Zoom:
					if (state->storages[SCORING + 1].usage > 0)
						state_set_source_colum(state, SCORING + 1);
				break;
		
				case sk_Trace:
					if (state->storages[SCORING + 2].usage > 0)
						state_set_source_colum(state, SCORING + 2);
				break;
		
				case sk_Graph:
					if (state->storages[SCORING + 3].usage > 0)
						state_set_source_colum(state, SCORING + 3);
				break;
		
				case sk_Yequ:
				case sk_0:
					if (state->storages[1].usage > 0)
						state_set_source_colum(state, DISCARD);
				break;
		
				case sk_1:
					if (state->storages[COLUM + 0].usage > 0)
						state_set_source_colum(state, COLUM + 0);
				break;
		
				case sk_2:
					if (state->storages[COLUM + 1].usage > 0)
						state_set_source_colum(state, COLUM + 1);
				break;
		
				case sk_3:
					if (state->storages[COLUM + 2].usage > 0)
						state_set_source_colum(state, COLUM + 2);
				break;
		
				case sk_4:
					if (state->storages[COLUM + 3].usage > 0)
						state_set_source_colum(state, COLUM + 3);
				break;
		
				case sk_5:
					if (state->storages[COLUM + 4].usage > 0)
						state_set_source_colum(state, COLUM + 4);
				break;
		
				case sk_6:
					if (state->storages[COLUM + 5].usage > 0)
						state_set_source_colum(state, COLUM + 5);
				break;
		
				case sk_7:
					if (state->storages[COLUM + 6].usage > 0)
						state_set_source_colum(state, COLUM + 6);
				break;
		
				case sk_Right:
					state_set_source_colum(state, state->selection_source + 1);
					if (state->selection_source >= CARD_STORAGE_NUMBER)
						state_set_source_colum(state, state->selection_source - CARD_STORAGE_NUMBER);
					while (state->storages[state->selection_source].usage == 0 || state->selection_source == 0) {
						state_set_source_colum(state, state->selection_source + 1);
						if (state->selection_source >= CARD_STORAGE_NUMBER)
							state_set_source_colum(state, state->selection_source - CARD_STORAGE_NUMBER);
					}
					state->selection_source_index = state->storages[state->selection_source].usage - 1;
				break;
		
				case sk_Left:
					state_set_source_colum(state, state->selection_source - 1);
					while (state->storages[state->selection_source].usage == 0 || state->selection_source == 0) {
						if (state->selection_source == 0)
							state_set_source_colum(state, CARD_STORAGE_NUMBER - 1);
						else
							state_set_source_colum(state, state->selection_source - 1);
					}
					state->selection_source_index = state->storages[state->selection_source].usage - 1;
				break;
		
				case sk_Down:
				if (state->selection_source_index < state->storages[state->selection_source].usage - 1 && (COLUM <= state->selection_source && state->selection_source < SCORING)) {
					if (state->storages[state->selection_source].redraw_frames == 0) state->storages[state->selection_source].redraw_frames = 1;
					++state->selection_source_index;
				}
				break;
		
				case sk_Up:
				if (state->selection_source_index > 0 && (COLUM <= state->selection_source && state->selection_source < SCORING)) {
					if (state->storages[state->selection_source].redraw_frames == 0) state->storages[state->selection_source].redraw_frames = 1;
					--state->selection_source_index;
				}
				break;

				case sk_Power:
					if (state_is_valid_selected_target(state, card_suit(state_selection_source_card(*state)) + SCORING)) {
						state_set_target_colum(state, card_suit(state_selection_source_card(*state)) + SCORING);
						state_perform_game_move(state);
						state_set_target_colum(state, 0);
					}
				break;
		
				case sk_2nd:
				case sk_Enter:
				if (state_is_valid_selected_source_index(state, state->selection_source_index) || state_is_sin_mode(*state)) {
					state_set_target_colum(state, state->selection_source);
					state->game_state = GAME_STATE_SELECT_TARGET;
				}
				break;
			}

		} break;

		case GAME_STATE_SELECT_TARGET: {
			switch (key) {
				case sk_Math:
					if (kd_is_autowinnable(state))
						state->game_state = GAME_STATE_AUTOWIN;
				break;

				case sk_Del:
				case sk_Alpha:
					state->game_state = GAME_STATE_SELECT_SOURCE;
					state_set_target_colum(state, 0);
				break;
				case sk_Window:
					state_set_target_colum(state, SCORING + 0);
				break;
		
				case sk_Zoom:
					state_set_target_colum(state, SCORING + 1);
				break;
		
				case sk_Trace:
					state_set_target_colum(state, SCORING + 2);
				break;
		
				case sk_Graph:
					state_set_target_colum(state, SCORING + 3);
				break;
		
				case sk_1:
					state_set_target_colum(state, COLUM + 0);
				break;
		
				case sk_2:
					state_set_target_colum(state, COLUM + 1);
				break;
		
				case sk_3:
					state_set_target_colum(state, COLUM + 2);
				break;
		
				case sk_4:
					state_set_target_colum(state, COLUM + 3);
				break;
		
				case sk_5:
					state_set_target_colum(state, COLUM + 4);
				break;
		
				case sk_6:
					state_set_target_colum(state, COLUM + 5);
				break;
		
				case sk_7:
					state_set_target_colum(state, COLUM + 6);
				break;
		
				case sk_Right:
					do {
						state_set_target_colum(state, state->selection_target + 1);
						if (state->selection_target >= CARD_STORAGE_NUMBER)
							state_set_target_colum(state, state->selection_target - CARD_STORAGE_NUMBER);
						if (state->selection_target != 0 && state->selection_target != 1)
							break;
					} while (true);
				break;
		
				case sk_Left:
					do {
						if (state->selection_target == 0)
							state_set_target_colum(state, CARD_STORAGE_NUMBER - 1);
						else
							state_set_target_colum(state, state->selection_target - 1);
						if (state->selection_target != 0 && state->selection_target != 1)
							break;
					} while (true);
				break;

				case sk_Power:
					if (state_is_valid_selected_target(state, card_suit(state_selection_source_card(*state)) + SCORING)) {
						state_set_target_colum(state, card_suit(state_selection_source_card(*state)) + SCORING);
					}
				break;

				case sk_2nd:
				case sk_Enter:
					if (state_is_valid_target(*state) || state_is_sin_mode(*state)) {
						state_perform_game_move(state);
					}
					state->game_state = GAME_STATE_SELECT_SOURCE;
					if (state->selection_target >= COLUM && state->selection_target < COLUM + 7)
						state_set_source_colum(state, state->selection_target);
					state->selection_source_index = state->storages[state->selection_source].usage - 1;
					state_set_target_colum(state, DECK);
				break;
			}
		} break;

		case GAME_STATE_AUTOWIN: {
			if (kd_is_won(state) && aq_has_room(state->animation_queue)) {
				state->game_state = GAME_STATE_WON;
				dbg_printf("set state->game_state = GAME_STATE_WON\n");
			} else {
				for (uint8_t source = DISCARD; source < COLUM + 7; ++source) {
					if (state->storages[source].usage > 0) {
						for (uint8_t target = SCORING; target < SCORING + 4; ++target) {
							if (
								( // correct suit
									card_suit(state_storage_top_card(*state, source)) == target - SCORING
								) && (
									(
										state->storages[target].usage == 0 // target empty
										&& (state_storage_top_card(*state, source).value == CARD_VALUE_ACE) // source is ace
									) || ( // target used and source == target + 1
										state->storages[target].usage > 0 // target has cards
										&& state_storage_top_card(*state, source).value == state_storage_top_card(*state, target).value + 1 // == target + 1
									)
								)
							) {
								dbg_printf("Moving card for autowin\n");
								cs_move_top_card_to_top(&state->storages[source], &state->storages[target], 6);
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
			//submit_animation(&state_storage_top_card(state, temp), rand() % (320 - CARD_WIDTH), rand() % (240 - CARD_HEIGHT), temp);
			if (key == sk_2nd) {
				state->game_state = GAME_STATE_WON;
			}
		} break;

		case GAME_STATE_WON: {
			switch (key) {
				case sk_2nd:
					state->game_state = GAME_STATE_NULL;
					state->program_state = PROGRAM_STATE_NULL;
				break;
			}
		} break;

		case GAME_STATE_NULL: {
			dbg_printf("This is very wrong game state null\n");
		} break;
	}
}

void kd_fx_clear(struct State* state) {
	// clear the screen areas that need redrawn
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		if (card_is_valid(state->animation_queue[i].card)) {
			dbg_printf("clearing animated card beck with life %d\n", state->animation_queue[i].card.life_remaining);
			gfx_Sprite_NoClip(
				state->animation_queue[i].behind_sprite,
				state->animation_queue[i].current_x,
				state->animation_queue[i].current_y
			);
		}
	}
	gfx_SetColor(0);
	if (state->storages[DECK].redraw_frames > 0) { // deck
		dbg_printf("Clearing Deck\n");
		gfx_FillRectangle_NoClip(0, 0, CARD_WIDTH + 2*12 + 4, CARD_HEIGHT + 4);
	}
	if (state->storages[DISCARD].redraw_frames > 0) { // discard
		dbg_printf("Clearing Discard\n");
		gfx_FillRectangle_NoClip(80 - 2, 0, CARD_WIDTH + 2*12 + 4, CARD_HEIGHT + 4);
	}
	for (uint8_t i = SCORING; i < SCORING + 4; ++i) { // scoring
		if (state->storages[i].redraw_frames > 0) {
			dbg_printf("Clearing Scoring %d\n", i);
			gfx_FillRectangle_NoClip(150 + (i - SCORING) * (CARD_WIDTH + 4) - 2, 0, CARD_WIDTH + 4, CARD_HEIGHT + 4);
		}
	}
	for (uint8_t colum = COLUM; colum < COLUM + 7; ++colum) { // colum
		if (state->storages[colum].redraw_frames > 0 ) {
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

void kd_fx_draw(struct State* state) {
	kd_fx_clear(state);

	// draw the deck
	kd_fx_draw_talon(state);

	// draw the discard pile
	kd_fx_draw_discard(state);

	// draw the scoring piles
	kd_fx_draw_foundations(state);

	// playfield
	kd_fx_draw_tableau(state);

	if (state_is_sin_mode(*state)) gfx_SetTextFGColor(COLOR_INVALID_SELECTION);
	else gfx_SetTextFGColor(COLOR_WHITE);
	gfx_SetTextScale(2, 2);
	gfx_PrintStringXY("Time: ", 5, 224);
	gfx_PrintInt((clock() - state->time_game_begin) / CLOCKS_PER_SEC, 1);
	gfx_PrintStringXY("Moves: ", 160, 224);
	gfx_PrintInt(state->game_move_count, 1);

	aq_render_and_animate_cards(state->animation_queue);

	gfx_BlitBuffer();
}

void kd_fx_draw_talon(struct State* state) {
	if (state->storages[DECK].redraw_frames > 0) {
		dbg_printf("Drawing Deck\n");
		--state->storages[DECK].redraw_frames;
		uint8_t num_to_draw = min(3, state->storages[DECK].usage);
		for (uint8_t i = 0; i < num_to_draw; ++i) {
			struct Card* card = &state->storages[DECK].data[state->storages[DECK].usage + i - num_to_draw];
			uint24_t x = 2 + i * 12;
			uint8_t y = 2;
			gpfx_draw_maybe_animated_card(state->animation_queue, card, x, y, &state->storages[DECK]);
		}
	}
}

void kd_fx_draw_discard(struct State* state) {
	if (state->storages[DISCARD].redraw_frames > 0) {
		dbg_printf("Drawing Discard\n");
		--state->storages[DISCARD].redraw_frames;
		uint8_t num_to_draw = min(3, state->storages[DISCARD].usage);
		for (uint8_t i = 0; i < num_to_draw; ++i) {
			struct Card* card = &state->storages[DISCARD].data[state->storages[DISCARD].usage + i - num_to_draw];
			uint24_t x = 80 + i * 12;
			uint8_t y = 2;
			gpfx_draw_maybe_animated_card(state->animation_queue, card, x, y, &state->storages[DISCARD]);
			if (DISCARD == state->selection_source && (state->storages[DISCARD].usage + i - num_to_draw) == state->selection_source_index) {
				gpfx_draw_highlight(card);
			}
		}
		if (state->selection_target == DISCARD) {
			gfx_SetColor((state_is_valid_target(*state)) ? COLOR_VALID_SELECTION : COLOR_INVALID_SELECTION);
			gfx_Rectangle_NoClip(
				78, 0,
				CARD_WIDTH + 4 + num_to_draw * 12 - 12,
				CARD_HEIGHT + 4
			);
		}
	}
}

void kd_fx_draw_tableau(struct State* state) {
	for (uint8_t colum = COLUM; colum < COLUM + 7; ++colum) {
		if (state->storages[colum].redraw_frames > 0 ) {
			--state->storages[colum].redraw_frames;
			dbg_printf("Drawing Colum %d with %d frames left\n", colum, state->storages[colum].redraw_frames);
			uint8_t coverd_card_height = min((160 - 4 - CARD_HEIGHT) / (state->storages[colum].usage - 1), 15);
			for (uint8_t card_index = 0; card_index < state->storages[colum].usage; ++card_index) {
				struct Card* card = &state->storages[colum].data[card_index];
				uint24_t x = (colum - COLUM) * (CARD_WIDTH + 6) + 2;
				uint8_t y = card_index * coverd_card_height + CARD_HEIGHT + 4 + 1 + 2;
				gpfx_draw_maybe_animated_card(state->animation_queue, card, x, y, &state->storages[colum]);
				if (colum == state->selection_source && card_index == state->selection_source_index) {
					gpfx_draw_highlight(card);
				}
			}
			if (colum == state->selection_target) {
				gfx_SetColor((state_is_valid_target(*state)) ? COLOR_VALID_SELECTION : COLOR_INVALID_SELECTION);
				gfx_Rectangle_NoClip(
					(colum - COLUM) * (CARD_WIDTH + 6),
					CARD_HEIGHT + 4 + 1,
					CARD_WIDTH + 4,
					CARD_HEIGHT + 4 + (max(state->storages[colum].usage, 1) - 1) * coverd_card_height
				);
			}
		}
	}
}

void kd_fx_draw_foundations(struct State* state) {
	for (uint8_t i = SCORING; i < SCORING + 4; ++i) {
		if (state->storages[i].redraw_frames > 0) {
			dbg_printf("Drawing scoring %d\n", i);
			--state->storages[i].redraw_frames;
			if (state->storages[i].usage > 0) {
				const uint24_t x = 150 + (i - SCORING) * (CARD_WIDTH + 4);
				const uint8_t y = 2;
				struct Card* card = &state_storage_top_card(*state, i);
				if (gpfx_draw_maybe_animated_card(state->animation_queue, card, x, y, &state->storages[i]) && state->storages[i].usage > 1) {
					gpfx_drawCard(&state->storages[i].data[state->storages[i].usage - 2]);
				} else if (i == state->selection_source && state->storages[i].usage - 1 == state->selection_source_index) {
					gpfx_draw_highlight(card);
				}
			}
			else {
				gpfx_monoMaskSprite(
				*(gfx_vbuffer + CARD_HEIGHT/2 - 4) + 150 + CARD_WIDTH/2 - 4 + 1 + (i - SCORING) * (CARD_WIDTH + 4),
				(7 << 8) + (5),
				data + 130 + 7 * (i - SCORING)
				);
			}
			if (i == state->selection_target) {
				gfx_SetColor((state_is_valid_target(*state)) ? COLOR_VALID_SELECTION : COLOR_INVALID_SELECTION);
				gfx_Rectangle_NoClip(150 - 2 + (i - SCORING) * (CARD_WIDTH + 4), 0, CARD_WIDTH + 4, CARD_HEIGHT + 4);
			}
		}
	}
}
