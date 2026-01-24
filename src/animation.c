#include <debug.h>
#include <graphx.h>
#include "animation.h"
#include "card.h"

void aq_clear(struct Animation* aq) {
	if (aq->storage) cs_clear(aq->storage);
	aq->current_x = 0;
	aq->current_y = 0;
	if (aq->behind_sprite) {
		free(aq->behind_sprite);
		aq->behind_sprite = NULL;
	}
	aq->card.flags = 0;
	aq->card.life_remaining = 0;
	aq->card.value = CARD_VALUE_INVALID;
}

void animate(struct Animation* animation) {
	animation->current_x = (animation->current_x + animation->card.target_x) / 2;
	animation->current_y = (animation->current_y + animation->card.target_y) / 2;
}

bool aq_submit_animation(struct Animation* aq, struct Card* card, CardStorage* sto, uint24_t x, uint8_t y) {
	dbg_printf("aq_submit_animation()\n");
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		if (card_is_invalid(aq[i].card)) {
			aq[i].card = *card;
			aq[i].current_x = x;
			aq[i].current_y = y;
			aq[i].storage = sto;
			return true;
		}
	}
	dbg_printf("aq_submit_animation() queue full, dropping animation\n");
	return false;
}

bool aq_has_room(struct Animation* aq) {
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		if (card_is_valid(aq[i].card)) {
			return true;
		}
	}
	return false;
}

void aq_render_and_animate_cards(struct Animation* aq) {
	// animate and get background sprites
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		if (card_is_valid(aq[i].card)) {
			dbg_printf("aq_render_and_animate_cards()   drawing card\n");
			animate(&aq[i]);
			gfx_GetSprite_NoClip(aq[i].behind_sprite, aq[i].current_x, aq[i].current_y);
		}
	}
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		if (card_is_valid(aq[i].card)) {
			struct Card temp = aq[i].card;
			temp.target_x = aq[i].current_x;
			temp.target_y = aq[i].current_y;
			card_dbg_print(temp);
			gpfx_drawCard(&temp);
			--aq[i].card.life_remaining;
			if (aq[i].card.life_remaining == 0) {
				dbg_printf("animate() redraw storage %d\n", (int)aq[i].storage);
				if (aq[i].storage->redraw_frames == 0) aq[i].storage->redraw_frames = 1;
				aq[i].card.value = CARD_VALUE_INVALID;
			}
		}
	}
}

void aq_init(struct Animation* aq) {
	dbg_printf("aq_init()\n");
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		aq[i].card.value = CARD_VALUE_INVALID;
		if (aq[i].behind_sprite == NULL) {
			aq[i].behind_sprite = gfx_MallocSprite(CARD_WIDTH, CARD_HEIGHT);
		}
	}
}