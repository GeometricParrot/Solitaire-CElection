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
	ld a, (hl)
loop:
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
	djnz loop


	pop de
	push hl
	push bc
	push iy
	push de
    ret