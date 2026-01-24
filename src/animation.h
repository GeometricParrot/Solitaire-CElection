#ifndef ANIMATION_H
#define ANIMATION_H

#include <ti/vars.h>
#include "card_storage.h"
#include "graphx.h"
#include "gpfx.h"

struct Card;

struct Animation {
	CardStorage* storage;
	uint8_t current_y;
	uint24_t current_x;
	gfx_sprite_t* behind_sprite;
	struct Card card;
};

#define AQ_CAPACITY 13

void aq_clear(struct Animation* aq);
void animate(struct Animation* animation);
bool aq_submit_animation(struct Animation* aq, struct Card* card, CardStorage* sto, uint24_t x, uint8_t y);
bool aq_has_room(struct Animation* aq);
void aq_render_and_animate_cards(struct Animation* aq);
void aq_init(struct Animation* aq);

#endif