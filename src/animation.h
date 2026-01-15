#ifndef ANIMATION_H
#define ANIMATION_H

#include <ti/vars.h>
#include "card_storage.h"
#include "graphx.h"

struct Card;
int drawCard(struct Card* card, bool highlight);

struct Animation {
	CardStorage* storage;
	uint8_t current_y;
	uint24_t current_x;
	gfx_sprite_t* behind_sprite;
	struct Card card;
};

#define AQ_CAPACITY 13
struct AnimationQueue {
	//uint8_t usage;
	struct Animation queue[AQ_CAPACITY];
};

void animate(struct Animation* animation);
bool aq_submit_animation(struct AnimationQueue* aq, struct Card* card, CardStorage* sto, uint24_t x, uint8_t y);
bool aq_has_room(struct AnimationQueue* aq);
void aq_render_and_animate_cards(struct AnimationQueue* aq);
void aq_init(struct AnimationQueue* aq);

#endif