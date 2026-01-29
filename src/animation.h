#ifndef ANIMATION_H
#define ANIMATION_H

#include <ti/vars.h>
#include "card_storage.h"
#include "graphx.h"
#include "gpfx.h"

struct Card;

// clear all
// draw static
// fetch dynamic back
// draw dynamic


struct Animation {
	// C clear requested, A -> active animation
	// 0000 00CA
	uint8_t flags;
	uint8_t current_y;
	uint24_t current_x;
	CardStorage* storage;
	gfx_sprite_t* behind_sprite;
	struct Card card;
};

#define AQ_CAPACITY 13

#define an_clear_requested(an) (((an).flags & 0b10) >> 1)
#define an_set_clear_requested(an, boo) ((an).flags = (((an.flags) & 0b11111101) | (boo << 1)))

#define an_is_active(an) ((an).flags & 0b1)
#define an_set_is_active(an, boo) ((an).flags = (((an.flags) & 0b11111110) | boo))

void an_clear(struct Animation* an);
void animate(struct Animation* an);

bool aq_submit_animation(struct Animation* aq, struct Card* card, CardStorage* sto, uint24_t x, uint8_t y);
bool aq_has_room(struct Animation* aq);
void aq_render_and_animate_cards(struct Animation* aq);
void aq_init(struct Animation* aq);

void aq_draw_back_sprites(struct Animation* aq);

void aq_queue_condense(struct Animation* aq);


#endif