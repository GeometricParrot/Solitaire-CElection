#ifndef KLONDIKE_H
#define KLONDIKE_H

#include <stdint.h>
#include <stdbool.h>

struct State;

void kd_init(struct State* state);
bool kd_is_autowinnable(struct State* state);
bool kd_is_won(struct State* state);
void kd_step(struct State* state, uint8_t key);
void kd_fx_clear(struct State* state);
void kd_fx_draw(struct State* state);
void kd_fx_draw_talon(struct State* state);
void kd_fx_draw_discard(struct State* state);
void kd_fx_draw_tableau(struct State* state);
void kd_fx_draw_foundations(struct State* state);

#endif