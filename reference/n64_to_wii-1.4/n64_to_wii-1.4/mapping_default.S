translate_default:

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
	sbrs r7, R7_BIT_UNSWAP_L_Z
	GCMAP(L)
	sbrc r7, R7_BIT_UNSWAP_L_Z
	GCMAP(Z)	; unswap Z/L
	
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
	sbrs r7, R7_BIT_UNSWAP_L_Z
	GCMAP(Z)
	sbrc r7, R7_BIT_UNSWAP_L_Z
	GCMAP(L)	; unswap Z/L
	
	ld r16, x+		;	N64 R
	GCMAP(R)

;;;;;;;; Sin and punishment mapping
	sbrs r7, R7_BIT_SIN
	rjmp no_c_lf_rt_to_gc_xy
	ldi xl, lo8(n64_workbuf)
	ldi xh, hi8(n64_workbuf)

	adiw xl, N64_OFF_C_LEFT
	ld r16, x
	GCMAP(Y)
	sbiw xl, N64_OFF_C_LEFT

	adiw xl, N64_OFF_C_RIGHT
	ld r16, x
	GCMAP(X)
no_c_lf_rt_to_gc_xy:



	; Now do the C-Stick Y axis using the N64 C pad
	ldi xl, lo8(n64_workbuf)
	ldi xh, hi8(n64_workbuf)
	rcall loadTxBufY

	adiw xl, 12		; N64 C pad UP
	ld r16, x
	tst r16
	brne C_up

	ldi xl, lo8(n64_workbuf)
	ldi xh, hi8(n64_workbuf)
	adiw xl, 13
	ld r16, x		; down
	tst r16
	brne C_dn
	ldi r16, 0x7f	; middle position
	rjmp do_Cupdn	; up or down not pressed
C_up:
	ldi r16, 0xff	; up position
	rjmp do_Cupdn
C_dn:
	ldi r16, 0x00	; down position
do_Cupdn:
	ldi r17, 0x80
	adiw yl, 40		; point to GC C-Stick Y value byte
lp_Cupdn:
	mov r15, r17
	and r15, r16
	st y+, r15		; store r17 or 0
	lsr r17
	brne lp_Cupdn

	; Now do the C-Stick X axis using the N64 L/R C pad
	sbrc r7,R7_BIT_SIN ; sin & p
	rjmp after_c_left_right

	ldi xl, lo8(n64_workbuf)
	ldi xh, hi8(n64_workbuf)
	rcall loadTxBufY
	adiw xl, 14		; left
	ld r16, x
	tst r16
	brne C_lf

	ldi xl, lo8(n64_workbuf)
	ldi xh, hi8(n64_workbuf)
	adiw xl, 15		; right
	ld r16, x		
	tst r16
	brne C_rt
	ldi r16, 0x7f	; middle position
	rjmp do_Clfrt	; up or down not pressed
C_lf:
	ldi r16, 0x00	; up position
	rjmp do_Clfrt
C_rt:
	ldi r16, 0xff	; down position
do_Clfrt:
	ldi r17, 0x80
	adiw yl, 32		; point to GC C-Stick X value byte
lp_Clfrt:
	mov r15, r17
	and r15, r16
	st y+, r15		; store r17 or 0
	lsr r17
	brne lp_Clfrt
after_c_left_right:

	
	ret

