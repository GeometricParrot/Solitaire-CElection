#ifndef GLOBAL_STATE_H
#define GLOBAL_STATE_H

#include <stddef.h>
#include <time.h>

#include "card_storage.h"
#include "animation.h"

#define DECK 0
#define DISCARD 1
#define COLUM 2
#define SCORING 9

#define CARD_STORAGE_NUMBER 13

enum PROGRAM_STATE {
	PROGRAM_STATE_NULL,
	PROGRAM_STATE_MAIN_MENU,
	PROGRAM_STATE_KLONDIKE_IN_GAME,
};

enum GAME_STATE {
	GAME_STATE_NULL,
	GAME_STATE_SELECT_SOURCE,
	GAME_STATE_SELECT_TARGET,
	GAME_STATE_AUTOWIN,
	GAME_STATE_WINNING_ANIMATION,
	GAME_STATE_WON,
};


struct State {
	// flags def:
	// T: valid_target
	// 0000'000T
	uint8_t flags;
	enum PROGRAM_STATE program_state;
	enum GAME_STATE game_state;
	size_t selection_source;
	size_t selection_source_index;
	size_t selection_target;
	size_t selection_target_index;
	size_t game_move_count;
	clock_t time_game_begin;
	clock_t time_frame_begin;
	struct Animation animation_queue[AQ_CAPACITY];
	CardStorage storages[CARD_STORAGE_NUMBER];
};

#define state_set_redraw(state, sto) if ((state).storages[sto].redraw_frames == 0) (state).storages[sto].redraw_frames = 1
#define state_is_valid_target(state) ((state).flags & 0b00000001)
#define state_set_is_valid_target(state, set) ((state).flags = ((state).flags & 0b11111110) | set)

#define state_selection_source_card(state) ((state).storages[(state).selection_source].data[(state).selection_source_index])
#define state_storage_top_card(state, index) (state).storages[index].data[(state).storages[index].usage - 1]


void state_set_target_colum(struct State* state, uint8_t colum);

void state_set_source_colum(struct State* state, uint8_t colum);

void state_storage_fill_with_cards(struct State* state, uint8_t storage);

void state_perform_game_move(struct State* state);

bool state_is_valid_selected_source_index(struct State* state, uint8_t index);

#endif