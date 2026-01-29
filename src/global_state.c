#include "global_state.h"
#include "debug.h"

bool state_is_valid_selected_target(struct State* state, uint8_t storage) {
	if (storage < COLUM || storage > SCORING + 3
		|| storage == state->selection_source
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
			return (
				// king is selected and moving to blank
				state_selection_source_card(*state).value == CARD_VALUE_KING
				&& (state->storages[storage].usage == 0)
			) || (
					// top target .value == top source .vaule + 1
					(state_storage_top_card(*state, storage).value) == state_selection_source_card(*state).value + 1
				&& // opposite color suit
					card_suit(state_storage_top_card(*state, storage)) != card_suit(state_selection_source_card(*state))
					&& card_suit(state_storage_top_card(*state, storage)) != card_opposite_suit(state_selection_source_card(*state))
				
			);
		break;

		case SCORING + 0:
		case SCORING + 1:
		case SCORING + 2:
		case SCORING + 3:
			return (
				// top card is selected
				state->selection_source_index == state->storages[state->selection_source].usage - 1
				&&
				// correct suit
				card_suit(state_selection_source_card(*state)) == storage - SCORING
				&&
				(
					( // target is empty and source is ace
						state->storages[storage].usage == 0
						&& (state_selection_source_card(*state).value == CARD_VALUE_ACE) // source is ace
					) || ( // target used and source == target + 1
						state_selection_source_card(*state).value == state_storage_top_card(*state, storage).value + 1
					)
				)
			);
		break;

		default:
			return false;
		break;
	}
	return false;
}

bool state_is_valid_selected_source_index(struct State* state, uint8_t index) {
	if (index >= state->storages[state->selection_source].usage) {
		return false;
	}
	switch (state->selection_source) {
		case DISCARD:
		case SCORING + 0:
		case SCORING + 1:
		case SCORING + 2:
		case SCORING + 3:
			return index == state->storages[state->selection_source].usage - 1;
		break;

		case COLUM + 0:
		case COLUM + 1:
		case COLUM + 2:
		case COLUM + 3:
		case COLUM + 4:
		case COLUM + 5:
		case COLUM + 6:
			return (
				index == state->storages[state->selection_source].usage - 1
				||
				card_facing(state->storages[state->selection_source].data[index]) == CARD_FACING_UP
			);
		break;
	}
	return false;
}

void state_set_target_colum(struct State* state, uint8_t colum) {
	state_set_redraw(*state, state->selection_target);
	state_set_redraw(*state, colum);
	state_set_is_valid_target(*state, state_is_valid_selected_target(state, colum));
	state->selection_target = colum;
	dbg_printf("setting target, valididy is %d\n", (int) state_is_valid_target(*state));
}

void state_set_source_colum(struct State* state, uint8_t colum) {
	state_set_redraw(*state, state->selection_source);
	state_set_redraw(*state, colum);
	state->selection_source = colum;
	state->selection_source_index = state->storages[colum].usage - 1;
}

void state_storage_fill_with_cards(struct State* state, uint8_t storage) {
	for (uint8_t i = 0; i < 52; ++i) {
		struct Card card;
		card.life_remaining = 0;
		card.flags = 0;
		card.target_x = 0;
		card.target_y = 0;
		card.value = (i % 13) + 1;
		card_set_suit(card, i/13);
		card_set_facing(card, CARD_FACING_DOWN);
		card_dbg_print(card);
		cs_insert_to_top_card(&state->storages[storage], card);
	}	
}

void state_perform_game_move(struct State* state) {
	++state->game_move_count;
	if (state->selection_source == DISCARD) {
		if (state->storages[DISCARD].usage >= 1) {
			state->storages[DISCARD].data[state->storages[DISCARD].usage - 1].life_remaining = 4;
		}
		if (state->storages[DISCARD].usage >= 2) {
			state->storages[DISCARD].data[state->storages[DISCARD].usage - 2].life_remaining = 4;
		}
	}
	cs_move_cards(
		&state->storages[state->selection_source], &state->storages[state->selection_target],
		state->selection_source_index, state->storages[state->selection_target].usage, 0, 8
	);
	state->selection_source_index = state->storages[state->selection_source].usage - 1;
	if (state->storages[state->selection_source].usage > 0)
		card_set_facing(state->storages[state->selection_source].data[state->selection_source_index], CARD_FACING_UP);
}

void state_update_source_index_to_last(struct State* state) {
	state->selection_source_index = state->storages[state->selection_source].usage - 1;
}

void state_clear(struct State* state) {
	state->flags = 0;
	state->program_state = PROGRAM_STATE_NULL;
	state->game_state = GAME_STATE_NULL;
	state->selection_source = 0;
	state->selection_source_index = 0;
	state->selection_target = 0;
	state->selection_target_index = 0;
	state->game_move_count = 0;
	state->time_game_begin = 0;
	state->time_frame_begin = 0;
	uint8_t i;
	for (i = 0; i < sizeof(state->animation_queue) / sizeof(struct Animation); ++i) {
		an_clear(&state->animation_queue[i]);
	}
	for (i = 0; i < sizeof(state->storages) / sizeof(CardStorage); ++i) {
		cs_clear(&state->storages[i]);
	}
}