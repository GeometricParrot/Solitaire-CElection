#include "gpfx.h"
#include "card.h"
#include "gfx/gfx.h"
#include <graphx.h>
#include "bit_sprites.h"
#include "card_storage.h"
#include "animation.h"

int gpfx_drawCard(struct Card* card) {
	// facedown
	if (card_facing((*card)) == CARD_FACING_DOWN) {
		gfx_TransparentSprite_NoClip(bard_backv2, card->target_x, card->target_y);
	}
	else {
		switch (card->value) {
			case CARD_VALUE_KING:
				gfx_TransparentSprite_NoClip(king, card->target_x, card->target_y);
			break;

			case CARD_VALUE_QUEEN:
				gfx_TransparentSprite_NoClip(queen, card->target_x, card->target_y);
			break;

			case CARD_VALUE_JACK:
				gfx_TransparentSprite_NoClip(jack, card->target_x, card->target_y);
			break;

			default:
			gfx_TransparentSprite_NoClip(blank_cardv2, card->target_x, card->target_y);
		}

		uint8_t* const glif1 = buffer_position(card->target_x + 2, card->target_y + 5);
		uint8_t* const glif2 = buffer_position(card->target_x + CARD_WIDTH - 2 - 8, card->target_y + CARD_HEIGHT - 10 - 5);
		uint8_t const value = card->value - 1;

		// number glifs
		gpfx_monoMaskSprite(
			glif1,
			(10 << 8) + (2 + card_suit((*card))),
			data + 10 * value
		);
		gpfx_monoMaskSprite_flipped(
			glif2,
			(10 << 8) + (2 + card_suit((*card))),
			data + 10 * value
		);
		uint24_t height_color = (7 << 8) + (2 + card_suit((*card)));
		const uint8_t* const glif = data + 130 + 7 * card_suit((*card));
		if (value < 10) {
			uint8_t const sum = (value*(value+1)) / 2;
			for (uint8_t i = 0; i <= value; ++i) {
				gpfx_monoMaskSprite(
					buffer_position(card->target_x + glif_locations_x[sum + i], card->target_y + glif_locations_y[sum + i]),
					height_color,
					glif
				);
			}
		}
		else {
			gpfx_monoMaskSprite(
				buffer_position(card->target_x + CARD_WIDTH - 3 - GLIF_SMALL_WIDTH, card->target_y + 5),
				height_color,
				glif
			);
			gpfx_monoMaskSprite_flipped(
				buffer_position(card->target_x + 1, card->target_y + CARD_HEIGHT - GLIF_SMALL_HEIGHT - 5),
				height_color,
				glif
			);
		}
	}

	return 0;
}

void gpfx_draw_highlight(struct Card* card) {
	gfx_SetColor(COLOR_UNKNOWN_SELECTION);
	gfx_Rectangle_NoClip(card->target_x - 2, card->target_y - 2, CARD_WIDTH + 4, CARD_HEIGHT + 4);
}

bool gpfx_draw_maybe_animated_card(struct AnimationQueue* aq, struct Card* card, uint24_t x, uint8_t y, CardStorage* cs) {
	uint24_t old_x = card->target_x;
	uint8_t old_y = card->target_y;
	card->target_x = x;
	card->target_y = y;
	if ((x != old_x || y != old_y) && card->life_remaining > 0) {
		aq_submit_animation(aq, card, cs, old_x, old_y);
		return true;
	}
	card->life_remaining = 0;
	gpfx_drawCard(card);
	return false;
}

void gpfx_drawShadowText(const char* str, uint8_t x, uint8_t y, uint8_t scale) {
	gfx_SetTextScale(scale, scale);
	gfx_SetTextFGColor(5);
	gfx_PrintStringXY(str, x + scale, y + scale);
	
	gfx_SetTextFGColor(COLOR_WHITE);
	gfx_PrintStringXY(str, x, y);
}