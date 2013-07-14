
translate_ssbb:
	; We have the data! Translate it for the gamecube
	;
	; X will be at the beginning of the N64 buffer and will increment
	; one byte at the time. Y will always point to the gamecube transmission
	; buffer and the std instruction will be used.

	ldi xl, lo8(n64_workbuf)
	ldi xh, hi8(n64_workbuf)
	rcall loadTxBufY
#define GCMAP(n)	std y+ GC_OFF_##n , r16

	; Button remapping
	ld r16, x+		; N64 A
	GCMAP(A)
	
	ld r16, x+		; N64 B
	GCMAP(B)
	
	ld r16, x+		; N64 Z
	GCMAP(L)
	
	ld r16, x+		; N64 Start
	GCMAP(START)
	
	ld r16, x+		; 	N64 Up
	GCMAP(D_UP)
	
	ld r16, x+		;	N64 Down
	GCMAP(D_DOWN)
	
	ld r16, x+		;	N64 Left
	GCMAP(D_LEFT)
	
	ld r16, x+		;	N64 Right
	GCMAP(D_RIGHT)
	
	adiw xl, 2		;	-- Skip the 2 N64 null bits --
	
	ld r16, x+		;	N64 L
	GCMAP(R)
	
	ld r16, x+		;	N64 R
	GCMAP(Z)

	ld r16, x+		; N64 C UP
	ld r16, x+		; N64 C DOWN
	GCMAP(X)

	;; Gamecube analogic C-Stick Y axis (Up/Down)
	ldi r16, 0x7f ; default: Middle position

	; N64 C pad UP moves C stick to 
	ldi xl, lo8(n64_workbuf)
	ldi xh, hi8(n64_workbuf)
	adiw xl, 12		; point X to N64 C pad UP
	ld r17, x
	tst r17
	breq alt_ssbb_not_up
	ldi r16, 0xff ; C stick direction: Up
alt_ssbb_not_up:

#if 0
	adiw xl, 1		; point X to N64 C pad down
	ld r17, x		; down
	tst r17
	breq alt_ssbb_not_down
	ldi r16, 0x00	; C stick direction: Down
alt_ssbb_not_down:
#endif

	; Expand byte to 8 bytes for output buffer
	ldi r17, 0x80
	adiw yl, 40		; point to GC C-Stick Y value byte
alt_ssbb_lp_Cupdn:
	mov r15, r17
	and r15, r16
	st y+, r15		; store r17 or 0
	lsr r17
	brne alt_ssbb_lp_Cupdn


	; Now do the C-Stick X axis using the N64 L/R C pad
	ldi r16, 0x7f	; default: Middle Position
	rcall loadTxBufY

	ldi xl, lo8(n64_workbuf)
	ldi xh, hi8(n64_workbuf)
	adiw xl, 14		; left
	ld r17, x
	tst r17
	breq alt_ssbb_not_left
	ldi r16, 0x00
alt_ssbb_not_left:

	ldi xl, lo8(n64_workbuf)
	ldi xh, hi8(n64_workbuf)
	adiw xl, 15		; right
	ld r17, x
	tst r17
	breq alt_ssbb_not_right
	ldi r16, 0xff
alt_ssbb_not_right:


	; Expand byte to 8 bytes for output buffer
alt_ssbb_do_Clfrt:
	ldi r17, 0x80
	adiw yl, 32		; point to GC C-Stick X value byte
alt_ssbb_lp_Clfrt:
	mov r15, r17
	and r15, r16
	st y+, r15		; store r17 or 0
	lsr r17
	brne alt_ssbb_lp_Clfrt

	ret


