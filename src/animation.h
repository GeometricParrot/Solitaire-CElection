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
	// R -> queue backgroud copy, A -> animation is active, S -> at end queue source storage redraw E-> redraw every frame
	// 0000 ESRA
	uint8_t flags;
	uint8_t current_y;
	uint24_t current_x;
	CardStorage* storage;
	gfx_sprite_t* behind_sprite;
	struct Card card;
};

#define AQ_CAPACITY 13

#define an_is_active(an) ((an).flags & 0b1)
#define an_set_is_active(an, boo) ((an).flags = (((an.flags) & 0b11111110) | boo))

#define an_queue_redraw_background(an) (((an).flags & 0b10) >> 1)
#define an_set_queue_redraw_background(an, boo) ((an).flags = (((an.flags) & 0b11111101) | (boo << 1)))

#define an_ending_queue_source_redraw(an) (((an).flags & 0b100) >> 2)
#define an_set_ending_queue_source_redraw(an, boo) ((an).flags = (((an.flags) & 0b11111011) | (boo << 2)))

#define an_queue_redraw_background_every_frame(an) (((an).flags & 0b1000) >> 3)
#define an_set_queue_redraw_background_every_frame(an, boo) ((an).flags = (((an.flags) & 0b11110111) | (boo << 3)))

void an_clear(struct Animation* an);
void animate(struct Animation* an);

bool aq_submit_animation(struct Animation* aq, struct Card* card, CardStorage* sto, uint24_t x, uint8_t y, uint8_t flags);
bool aq_has_room(struct Animation* aq);
void aq_render_and_animate_cards(struct Animation* aq);
void aq_init(struct Animation* aq);

void aq_draw_back_sprites(struct Animation* aq);

void aq_queue_condense(struct Animation* aq);


#endif