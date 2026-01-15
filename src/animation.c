#include <debug.h>
#include <graphx.h>
#include "animation.h"
#include "card.h"


void animate(struct Animation* animation) {
	animation->current_x = (animation->current_x + animation->card.target_x) / 2;
	animation->current_y = (animation->current_y + animation->card.target_y) / 2;
}

bool aq_submit_animation(struct AnimationQueue* aq, struct Card* card, CardStorage* sto, uint24_t x, uint8_t y) {
	dbg_printf("aq_submit_animation()\n");
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		if (card_is_invalid(aq->queue[i].card)) {
			aq->queue[i].card = *card;
			aq->queue[i].current_x = x;
			aq->queue[i].current_y = y;
			aq->queue[i].storage = sto;
			return true;
		}
	}
	dbg_printf("aq_submit_animation() queue full, dropping animation\n");
	return false;
}

bool aq_has_room(struct AnimationQueue* aq) {
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		if (card_is_valid(aq->queue[i].card)) {
			return true;
		}
	}
	return false;
}
#include "gfx/gfx.h"
void aq_render_and_animate_cards(struct AnimationQueue* aq) {
	// animate and get background sprites
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		if (card_is_valid(aq->queue[i].card)) {
			dbg_printf("aq_render_and_animate_cards()   drawing card\n");
			animate(&aq->queue[i]);
			gfx_GetSprite_NoClip(aq->queue[i].behind_sprite, aq->queue[i].current_x, aq->queue[i].current_y);
		}
	}
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		if (card_is_valid(aq->queue[i].card)) {
			struct Card temp = aq->queue[i].card;
			temp.target_x = aq->queue[i].current_x;
			temp.target_y = aq->queue[i].current_y;
			card_dbg_print(temp);
			drawCard(
				&temp,
				false
			);
			--aq->queue[i].card.life_remaining;
			if (aq->queue[i].card.life_remaining == 0) {
				dbg_printf("animate() redraw storage %d\n", (int)aq->queue[i].storage);
				if (aq->queue[i].storage->redraw_frames == 0) aq->queue[i].storage->redraw_frames = 1;
				aq->queue[i].card.value = CARD_VALUE_INVALID;
			}
		}
	}
}

void aq_init(struct AnimationQueue* aq) {
	dbg_printf("aq_init()\n");
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		aq->queue[i].card.value = CARD_VALUE_INVALID;
		if (aq->queue[i].behind_sprite == NULL) {
			aq->queue[i].behind_sprite = gfx_MallocSprite(CARD_WIDTH, CARD_HEIGHT);
		}
	}
}