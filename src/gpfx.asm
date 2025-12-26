	assume  adl=1

    section .text

    public  _gpfx_monoMaskSprite
_gpfx_monoMaskSprite:
	pop de

    pop iy ; buffer pointer
	pop bc ; count-color
	pop hl ; data pointer
	push de
	
	; c contains color, b contains byte count
	; iy destination, 
	ld de, 320
_gpfx_monoMaskSprite_loop:
	ld a, (hl)
	bit 7, a
	jr z, skip_0
	ld (iy), c
skip_0:
	bit 6, a
	jr z, skip_1
	ld (iy + 1), c
skip_1:
	bit 5, a
	jr z, skip_2
	ld (iy + 2), c
skip_2:
	bit 4, a
	jr z, skip_3
	ld (iy + 3), c
skip_3:
	bit 3, a
	jr z, skip_4
	ld (iy + 4), c
skip_4:
	bit 2, a
	jr z, skip_5
	ld (iy + 5), c
skip_5:
	bit 1, a
	jr z, skip_6
	ld (iy + 6), c
skip_6:
	bit 0, a
	jr z, skip_7
	ld (iy + 7), c
skip_7:
	inc hl
	add iy, de
	djnz _gpfx_monoMaskSprite_loop

	pop de
	push hl
	push bc
	push iy
	push de
    ret


	assume  adl=1

    section .text
    public  _gpfx_monoMaskSprite_flipped
_gpfx_monoMaskSprite_flipped:
	pop de

    pop iy ; screen buffer pointer
	pop bc ; count-color
	pop hl ; data pointer
	push de
	
	; c contains color, b contains byte count
	; iy destination, 
	ld de, 0
	ld e, b
	dec e
	adc hl, de
	ld de, 320
_gpfx_monoMaskSprite_flipped_loop:
	ld a, (hl)
	bit 0, a
	jr z, skip_0_flipped
	ld (iy), c
skip_0_flipped:
	bit 1, a
	jr z, skip_1_flipped
	ld (iy + 1), c
skip_1_flipped:
	bit 2, a
	jr z, skip_2_flipped
	ld (iy + 2), c
skip_2_flipped:
	bit 3, a
	jr z, skip_3_flipped
	ld (iy + 3), c
skip_3_flipped:
	bit 4, a
	jr z, skip_4_flipped
	ld (iy + 4), c
skip_4_flipped:
	bit 5, a
	jr z, skip_5_flipped
	ld (iy + 5), c
skip_5_flipped:
	bit 6, a
	jr z, skip_6_flipped
	ld (iy + 6), c
skip_6_flipped:
	bit 7, a
	jr z, skip_7_flipped
	ld (iy + 7), c
skip_7_flipped:
	dec hl
	add iy, de
	djnz _gpfx_monoMaskSprite_flipped_loop

	pop de
	push hl
	push bc
	push iy
	push de
    ret