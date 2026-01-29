#include <debug.h>
#include <graphx.h>
#include "animation.h"
#include "card.h"
#include <string.h>

void an_clear(struct Animation* an) {
	if (an->storage) cs_clear(an->storage);
	an->flags = 0;
	an->current_x = 0;
	an->current_y = 0;
	if (an->behind_sprite) {
		free(an->behind_sprite);
		an->behind_sprite = NULL;
	}
	an->card.flags = 0;
	an->card.life_remaining = 0;
	an->card.value = CARD_VALUE_INVALID;
}

void animate(struct Animation* an) {
	an->current_x = (an->current_x + an->card.target_x) / 2;
	an->current_y = (an->current_y + an->card.target_y) / 2;
}

void aq_queue_condense(struct Animation* aq) {
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		if (!an_is_active(aq[i]) && !an_clear_requested(aq[i])) {
			// empty space found

			for (uint8_t j = i; j < AQ_CAPACITY; ++j) {
				if (an_is_active(aq[i]) && an_clear_requested(aq[i])) {
					// valid animation found after empty space

					memmove(aq + i, aq + j, (j-i) * sizeof(struct Animation));
					break;
				}
			}
		}
	}
}

// x is starting x, y is starting y
bool aq_submit_animation(struct Animation* aq, struct Card* card, CardStorage* sto, uint24_t x, uint8_t y) {
	dbg_printf("an_submit_animation()\n");
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		if (!an_is_active(aq[i]) && !an_clear_requested(aq[i])) {
			aq[i].card = *card;
			aq[i].current_x = x;
			aq[i].current_y = y;
			aq[i].storage = sto;
			an_set_is_active(aq[i], true);
			return true;
		}
	}
	dbg_printf("an_submit_animation() queue full, dropping animation\n");
	return false;
}

bool aq_has_room(struct Animation* aq) {
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		if (!an_is_active(aq[i])) {
			return true;
		}
	}
	return false;
}

void aq_render_and_animate_cards(struct Animation* aq) {
	// animate and get background sprites
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		if (an_is_active(aq[i])) {
			dbg_printf("an_render_and_animate_cards()   drawing card\n");
			animate(&aq[i]);
			gfx_GetSprite_NoClip(aq[i].behind_sprite, aq[i].current_x, aq[i].current_y);
			an_set_clear_requested(aq[i], true);

			struct Card temp = aq[i].card;
			temp.target_x = aq[i].current_x;
			temp.target_y = aq[i].current_y;
			card_dbg_print(temp);
			gpfx_drawCard(&temp);
			--aq[i].card.life_remaining;
			if (aq[i].card.life_remaining == 0) {
				if (aq[i].storage != NULL && aq[i].storage->redraw_frames == 0) {
					dbg_printf("animate() redraw storage %d\n", (int)aq[i].storage);
					aq[i].storage->redraw_frames = 1;
				}
				an_set_is_active(aq[i], false);
			}
		}
	}
}

void aq_init(struct Animation* aq) {
	dbg_printf("an_init()\n");
	for (uint8_t i = 0; i < AQ_CAPACITY; ++i) {
		aq[i].card.value = CARD_VALUE_INVALID;
		if (aq[i].behind_sprite == NULL) {
			aq[i].behind_sprite = gfx_MallocSprite(CARD_WIDTH, CARD_HEIGHT);
		}
	}
}

void aq_draw_back_sprites(struct Animation* aq) {
	for (uint8_t i = AQ_CAPACITY - 1; i < AQ_CAPACITY; --i) {
		if (an_clear_requested(aq[i])) {
			dbg_printf("clearing animated card back with life %d\n", aq[i].card.life_remaining);
			gfx_Sprite_NoClip(
				aq[i].behind_sprite,
				aq[i].current_x,
				aq[i].current_y
			);
			an_set_clear_requested(aq[i], false);
		}
	}
}