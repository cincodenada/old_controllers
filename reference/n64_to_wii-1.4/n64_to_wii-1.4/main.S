;
;	n64_to_wii: AVR Microcontroller firmware for converting N64 controller
;	          protocol to Nintendo Wii/Gamecube controller protocol.
;    Copyright (C) 2007-2008  Raphael Assenat <raph@raphnet.net>
;
;    This program is free software; you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation; either version 2 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License along
;    with this program; if not, write to the Free Software Foundation, Inc.,
;    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
;
;	------------------------------------------------------------------------   
;
;	Register usages:
;
;	------ read only registers 
;		r0: Constant 0
;
;	------ registers shared between interrupt and non-interrupt context
;		r6: Select current GC reply buffer
;		r3: Set to non-zero after each interrupt, used to read from the slave
;		controller immediately after each interrupt.
;		r4-r5: Nes/Snes controller latest read bytes (r4 filled first)
;		r7: Mapping tweaks register bitfield.
;				0: If set, unswap L and Z buttons     		(R7_BIT_UNSWAP_L_Z)
;               1: Sin and punishment mapping {				(R7_BIT_SIN)
;						N64 C-Stick Left -> Gamecube Y	
;						N64 C-Stick Right -> Gamecube X
;				}
;				2: Disable N64 joystick value scaling		(R7_BIT_NOSCALE)
;				3: If set, scale by 1.5 instead of 1.375 (if scaling is not disabled)
;															(R7_BIT_SCALE_1_5)
;
;	------ registers for non-interrupt context
;		r15: Argument or temporary
;		r16: Temporary register for non-interrupt context
;		r17: Loop counter for reading from controller
;		r18: Temporary storage while reading from controller
;		r21: Error flag for N64 data reception
;
;	------ Interrupt context registers (some are used before enabling interrupts though)
;		r1: SREG backup in interrupt. Faster than pushing it to stack.
;		r19: Temporary register for interrupt context
;		r20: Temporary register for interrupt context
;		r23: Bit Counter for interrupt context
;		r8: Variable. When non-zero, reply with 80 bits instead of 64.

#include <avr/io.h>
#include "gc_offsets.h"
#include "n64_offsets.h"

#define R7_BIT_UNSWAP_L_Z	0
#define R7_BIT_SIN			1
#define R7_BIT_NOSCALE		2
#define R7_BIT_SCALE_1_5	3
#define R7_BIT_ALT_1		4
#define R7_BIT_ALT_2		5
#define R7_BIT_ALT_3		6
#define R7_BIT_ALT_4		7

.lcomm gc_rx_buf, 25
.lcomm gc_tx_buf0, 64	; reply is 64 bit + 1 stop bit
.lcomm gc_tx_buf0_extra, 16	; for 80bit responses
.lcomm gc_tx_buf1, 64	;
.lcomm gc_tx_buf1_extra, 16 ; for 80bit response
.lcomm n64_workbuf, 34	; read/write buffer for N64 comm
.lcomm id_status, 24	; reply to 0000 0000 1 (0x00)
.lcomm origins_buf, 80	; reply to 0100 0001 1 (0x41)

.text
.global main
.global SIG_INTERRUPT0
;.global gc_rx_buf
.global n64_workbuf
.global loadTxBufY

#define IO_SREG	_SFR_IO_ADDR(SREG)
#define IO_PORTD _SFR_IO_ADDR(PORTD)
#define IO_DDRD _SFR_IO_ADDR(DDRD)
#define IO_PIND	_SFR_IO_ADDR(PIND)
#define IO_PORTC _SFR_IO_ADDR(PORTC)
#define IO_DDRC _SFR_IO_ADDR(DDRC)
#define IO_PINC _SFR_IO_ADDR(PINC)
#define IO_PORTB _SFR_IO_ADDR(PORTB)
#define IO_DDRB _SFR_IO_ADDR(DDRB)
#define IO_PINB	_SFR_IO_ADDR(PINB)
#define IO_MCUCR _SFR_IO_ADDR(MCUCR)
#define IO_GICR	_SFR_IO_ADDR(GICR)

#define CLK_H_LATCH_H	0x30
#define CLK_H_LATCH_L	0x20
#define CLK_L_LATCH_L	0x00

#define DEBUG_PORTD_BIT 0x00	/* In PORTD */
#define GC_DATA_BIT		0x02	/* in PORTD */
#define GC_DATA_MASK	0x04	
#define DATA_FALL_TIMEOUT	0x15
#define DATA_RAISE_TIMEOUT	0x15 

; Useful
#define xl	r26
#define xh	r27
#define yl	r28
#define yh	r29
#define zl	r30
#define zh	r31

SIG_INTERRUPT0:
	in r1, IO_SREG
	ldi	zl, lo8(gc_rx_buf)
	ldi zh, hi8(gc_rx_buf)
	ldi r23, 25				; We will receive a 24bit command

	rjmp fell


readNextBit:
	ldi r19, DATA_FALL_TIMEOUT	; setup a timeout
waitFall:				
	dec r19						; 1
	breq timeout				; 1
	in r20, IO_PIND				; 1 : read the input port
	andi r20, GC_DATA_MASK		; 1 : isolate the input bit
	brne waitFall				; 2 : if still high, loop

	; When we first jump to 'fell' (when the interrupt
	; occurs), we have already wasted many cycles. Those
	; delays are there to compensate and make sure we
	; always sample the data line where we want.
	nop
	nop
	nop
	nop
	nop
	nop
fell:

	; Ok, so there is now a 0 on the wire. 
	;  Worst case, we are at the 9th cycle.
	;  Best case, we are at the 3rd cycle.
	;  Lets assumbe we are at cycle 6.

	;  cycle: 1-16 16-32 32-48 48-64
	;   high:  0     1     1     1
	;   low:  0     0     0     1

	;  I check the pin on the 32th cycle which is 
	;  the safest place. Assuming we are on the 6th cycle,
	;  we need to waste 26 cycles. This is done by
	;  adjusting the align_delay subroutine.

	rcall align_delay				; waste enough cycles to be aligned on cycle 32
	sbi IO_PORTD, DEBUG_PORTD_BIT	; DEBUG
	in r20, IO_PIND					; read the input port
	cbi IO_PORTD, DEBUG_PORTD_BIT	; DEBUG
	andi r20, GC_DATA_MASK			; isolate the data bit
	st z+, r20						; store the value

	dec r23							; One less bit to receive...
	breq gotCommand

	ldi r19, DATA_RAISE_TIMEOUT
waitHigh:
	dec r19
	breq timeout
	in r20, IO_PIND
	andi r20, GC_DATA_MASK
	breq waitHigh					; Still high? Keep looping

	rjmp readNextBit

	; At this point, we have read a full 24bit command. A document
	; I found says that there is a 6ms second delay before a real gamecube
	; controller responds. That leaves us a lot of time :)
gotCommand:
	ldi	zl, lo8(gc_rx_buf)
	ldi zh, hi8(gc_rx_buf)

	; note: Last byte not checked... it contains a bit for rumble motor.. I'm 
	; not sure about the others. I'd rather reply anyway than not doing it and
	; having the console beleiving the controller is gone or failing... 

	; Here are some possible 24 bit commands and the reply length assotiated with them.
	; 64 bit replies: 0x400300, 0x400302  ...
	; 80 bit replies: 0x420000, 0x420302

	; test only bit 6 (0x020000) since it is apparently when this 
	; very bit is set that a controller replies with 80 bits 
	ldd r8, Z+6

	rjmp sendLatestBuffer

timeout:
	;  If we have received only 9 bits, the command is possibly getID+status. We
	;   _must_ respond to this if we want the console to detect us. 
	;
	; r23 is a count-down from 25 during the receive phase so it should
	; contain a value of 16.. 
	ldi r19, 16
	sub r19, r23 
	brne interrupt_done

	; point z to the first bit (one byte per bit) of the
	; message we just received. 
	ldi zl, lo8(gc_rx_buf)
	ldi zh, hi8(gc_rx_buf)

	; Commands: 0x00 = Get ID + Status, 0x41 = Get origins? 
;
	; Check that bits 7,5-0 are low. Then, if bit 6 is high,
	; command is 0x41. Else, it is 0x00.
	;
	ld r19, z+	;	bit 7
	tst r19
	brne unknown_cmd
	ld r19, z+	;	bit 6 (ignored for now)
	ld r19, z+	;	bit 5
	tst r19
	brne unknown_cmd
	ld r19, z+	;	bit 4
	tst r19
	brne unknown_cmd
	ld r19, z+	;	bit 3
	tst r19
	brne unknown_cmd
	ld r19, z+	;	bit 2
	tst r19
	brne unknown_cmd
	ld r19, z+	;	bit 1
	tst r19
	brne unknown_cmd
	
	ld r19, z+	;	bit 0
	tst r19
	clc
	breq checkBit6
	sec

checkBit6:
	; now check bit 6
	ldi zl, lo8(gc_rx_buf)	; point z to
	ldi zh, hi8(gc_rx_buf)	; 			bit 7
	ld r19, z+				;	advance to bit 6
	ld r19, z
	tst r19
	breq not_41		;	 if it's clear, then we got the 0x00 command.
	; ok, so it is 0x40 or 0x41.
	brcc unknown_cmd
	; Bit was set? It's 0x40!
	rjmp sendOriginsPacket

not_41: ; we jumped here beacuse bit 6 is 0. Bit 0 must be low too.
	brcs unknown_cmd	; 0x01 is not a command we know about. 
	rjmp sendIdPacket

unknown_cmd:
interrupt_done:
	out IO_SREG, r1
	reti

interrupt_done_poll:
	inc r3				; when r3 is non-zero, mainloop reads one time from the
						; slave controller and clears this register.
	out IO_SREG, r1
	reti

align_delay:
	ldi r19, 3
lp634534:
	dec r19
	brne lp634534
	ret

sendIdPacket:
	ldi	zl, lo8(id_status)
	ldi zh, hi8(id_status)
	ldi r23, 24
	rcall sendGCData
	rjmp interrupt_done

sendOriginsPacket:
	ldi	zl, lo8(origins_buf)
	ldi zh, hi8(origins_buf)
	ldi r23, 80
	rcall sendGCData
	rjmp interrupt_done


	/******************************************
	Load the latest buffer address in Z and
	send it.
	******************************************/
sendLatestBuffer:
	tst r6
	brne useBuf1

	; Select the buffer
	ldi	zl, lo8(gc_tx_buf0)
	ldi zh, hi8(gc_tx_buf0)
	rjmp bufferSelected
useBuf1:
	ldi	zl, lo8(gc_tx_buf1)
	ldi zh, hi8(gc_tx_buf1)	
bufferSelected:

	; When r8 != 0, send a 80 bit reply 
	tst r8
	breq send64Bits
	ldi r23, 80
	rjmp dosendGCData

send64Bits:
	; Send the packet
	ldi r23, 64
dosendGCData:
	rcall sendGCData
	rjmp interrupt_done_poll


	/************************************************
	* Send data using the N64/GC serial protocol which
	* is as follows:
	*   0          1
	*     __        _____
	* ____|       __|
	* ^   ^ ^     ^ ^   ^
	*  3us 1us    1us 3us
	*
	* To send a 1, the pin direction is set to input.
	* To send a 0, the pin direction is set to output. 
	*	(of course, it's value is preset to zero)
	* 
	* At 16 mhz, a 1us period is 16 cycles. Thus a 3us period
	* is 48 cycles.
	*
	* Number of bits to send is passed in r23
	* Z must point to first byte of data. Every byte
	* represents one bit (programming laziness..). A logic
	* 1 is sent when a byte is non-zero.
	*
	* A stop bit is added at thy end of the packet.
	*
	* Used registers: r19, r20
	************************************************/
sendGCData:

lp_sendGCData_bits:
	ld r19, z+
	tst r19
	brne send1
	nop
send0:
	sbi IO_DDRD, GC_DATA_BIT	; Pull bus to 0 
	
	ldi r20, 15
lp_send0_3us:
	dec r20
	brne lp_send0_3us
	nop

	cbi IO_DDRD, GC_DATA_BIT	; Release bus to 1

	ldi r20, 2
lp_send0_1us:
	dec r20
	brne lp_send0_1us

	dec r23
	brne lp_sendGCData_bits

	nop
	nop
	nop
	nop
	nop
	nop
	; STOP BIT
	sbi IO_DDRD, GC_DATA_BIT ; Pull low for stop bit
	ldi r20, 4
stbdly0:
	dec r20
	brne stbdly0
	nop
	cbi IO_DDRD, GC_DATA_BIT ;	Release

	ret

send1:
	sbi IO_DDRD, GC_DATA_BIT	; Pull bus to 0

	ldi r20, 4
lp_send1_1us:
	dec r20
	brne lp_send1_1us
	nop
	nop
	
	cbi IO_DDRD, GC_DATA_BIT	; Release bus to 1

	ldi r20, 12
lp_send1_3us:
	dec r20
	brne lp_send1_3us
	nop
	nop

	dec r23
	brne lp_sendGCData_bits

	nop
	nop
	nop
	nop
	nop
	nop

	; STOP BIT
	sbi IO_DDRD, GC_DATA_BIT ; Pull low for stop bit
	ldi r20,4
stbdly1:
	dec r20
	brne stbdly1
	nop
	cbi IO_DDRD, GC_DATA_BIT	; Release
	
	ret

	 /*
	 * -- PORTD --
	 * 2: Int0 (GC Data In (simulated open collector by toggling direction))
	 * 1: NC or Shorted with 0
	 * 0: Debug bit
	 */
main:
	clr r0

	/* Configure Latch and Clock as outputs and
	   enable pull-ups on inputs/unused pins,  */
	ldi r16, 0xe7	/* 1110 0111 */
	out IO_PORTC, r16
	ldi r16, 0x30	/* 0011 0000 */
	out IO_DDRC, r16

	/* Configure Jumpers as inputs with pull-ups. 
	   Other PORTB pins are ISP pins. Configure those
	   in the same way. */
	ldi r16, 0xff
	out IO_PORTB, r16
	ldi r16, 0x00
	out IO_DDRB, r16

	/* All PORTD as input with pullups everywhere, except
	   the Int0 (PD2) pin. The later must not have a pullup
	   for we dont want to send 5volts to the host (Gamecube, Wii or
	   compatible adapter. Bit0 is also reserved for debugging 
	   purposes. It is set as output, no pullup.  */
	ldi r16, 0xfa 	/* 1111 1010 */
	out IO_PORTD, r16
	ldi r16, 0x01	/* 0000 0001 */
	out IO_DDRD, r16

	/* Configure external interrupt INT0 (falling edge) */
	ldi r16, 0x02	/* 0000 0010 */ 
	out IO_MCUCR, r16
	ldi r16, 0x40	/* INT0 set */
	out IO_GICR, r16


	rcall clr_precooked_buffers
	rcall build_id_packet
	rcall build_origins_packet	; must be called after clr_precooked_buffers
	rcall initReplyBuf

	;inc r7 ; test

	clr r6 	; select buffer 0 initially 
	clr r3

	; before enabling interrupt, read from the controller. Depending
	; on which buttons are down, enable some mapping tweaks.
	;
	; readN64 sets r15 to 0 on success.
retryRead:
	rcall delay_16ms
	rcall readN64
	tst r21
	brne retryRead

	clr r7	; disable all mapping tweaks
	ldi yl, lo8(n64_workbuf)
	ldi yh, hi8(n64_workbuf)

	;;;; L pressed at startup. Dont swap Z and L buttons.
	ldd r16, y+N64_OFF_L
	tst r16
	breq no_unswap
	ldi r16, (1<<R7_BIT_UNSWAP_L_Z)	; bit0: unswap
	or r7, r16
no_unswap:

	;;;; R pressed at startup. Sin and punishment mapping.
	ldd r16, y+N64_OFF_R
	tst r16
	breq no_sinp
	ldi r16, (1<<R7_BIT_SIN)	; bit1: sin and punishment mapping
	or r7, r16
no_sinp:

	;;;; D pad Left pressed at startup. Disable value scaling.
	ldd r16, y+N64_OFF_D_LEFT
	tst r16
	breq no_disable_scaling
	ldi r16, (1<<R7_BIT_NOSCALE)	; bit2: Disable N64 joystick value scaling
	or r7, r16
no_disable_scaling:

	;;;; D pad Right pressed at startup. Scale N64 values by 1.5 instead of 1.375
	ldd r16, y+N64_OFF_D_RIGHT
	tst r16
	breq no_1dot5_scaling
	ldi r16, (1<<R7_BIT_SCALE_1_5)	; bit3: Scale N64 values by 1.5 if set.
	or r7, r16
no_1dot5_scaling:


	;;;; Take the C buttons as bits
	ldd r16, y+N64_OFF_C_UP
	tst r16
	breq no_c_up
	ldi r16, (1<<R7_BIT_ALT_1)	; bit3: Scale N64 values by 1.5 if set.
	or r7, r16
no_c_up:
	ldd r16, y+N64_OFF_C_DOWN
	tst r16
	breq no_c_down
	ldi r16, (1<<R7_BIT_ALT_2)	; bit3: Scale N64 values by 1.5 if set.
	or r7, r16
no_c_down:
	ldd r16, y+N64_OFF_C_LEFT
	tst r16
	breq no_c_left
	ldi r16, (1<<R7_BIT_ALT_3)	; bit3: Scale N64 values by 1.5 if set.
	or r7, r16
no_c_left:
	ldd r16, y+N64_OFF_C_RIGHT
	tst r16
	breq no_c_right
	ldi r16, (1<<R7_BIT_ALT_4)	; bit3: Scale N64 values by 1.5 if set.
	or r7, r16
no_c_right:





	sei
mainloop:
	tst r3				; set inside int handler
	breq mainloop;

	rcall readN64
	rjmp dly
	
dly:

	; Note: SLOW_POLLING is not defined. The Wii polls two times at 1ms intervals,
	; followed by an approx. delay of 16ms before the next two reads. Why? Debouncing maybe?
	; In our case, this permits a more up-to-date button state on the second read because
	; we will report freshly read N64 controller data on the second read. (that is, if SLOW_POLLING
	; is not defined.... Too bad for non-standard controllers if it cause a problem.
#ifdef SLOW_POLLING 
	rcall delay_16ms	; dont read the controller again too soon?
						; just in case some non-standard compatible
						; controller does not like this.
#endif
	clr r3
	rjmp mainloop


	/*************************************************************
	 READ N64
	 */
readN64:
	
	/* Get button/axes report command is 0x0000 0001 */
	ldi yl, lo8(n64_workbuf)
	ldi yh, hi8(n64_workbuf)
	ldi r16, 0x01
	st y+, r0
	st y+, r0
	st y+, r0
	st y+, r0
	
	st y+, r0
	st y+, r0
	st y+, r0
	st y+, r16
	st y+, r16 ; stop bit
	
	ldi yl, lo8(n64_workbuf)
	ldi yh, hi8(n64_workbuf)

	ldi r16, 9 ; byte + stop bit
	mov r15, r16
	rcall n64_sendData
	; n64_sendData returns non-zero in r15 if an error occurs.
	tst r15
	breq translate

	ret

loadTxBufY:
	; When r6 is non-zero, interrupt context
	; reads from buf1. This means that we should
	; write to buf0.
	tst r6
	brne useBuf0

	ldi yl, lo8(gc_tx_buf1)
	ldi yh, hi8(gc_tx_buf1)

	ret

useBuf0:
	ldi yl, lo8(gc_tx_buf0)
	ldi yh, hi8(gc_tx_buf0)

	ret

translate:
	; We have the data! Translate it for the gamecube
	;
	; X will be at the beginning of the N64 buffer and will increment
	; one byte at the time. Y will always point to the gamecube transmission
	; buffer and the std instruction will be used.
	mov r17, r7
	andi r17, 0xf0

test_map_1:
	ldi r16, 0x10
	eor r16, r17
	tst r16
	brne test_map2
	rcall translate_ssbm	
	rjmp translate_common

test_map2:
	ldi r16, 0x20
	eor r16, r17
	tst r16
	brne test_map4
	rcall translate_ssbm2
	rjmp translate_common
	
test_map4:
	ldi r16, 0x40
	eor r16, r17
	tst r16
	brne test_map8
	rcall translate_ssbb
	rjmp translate_common
	
test_map8:
	ldi r16, 0x80
	eor r16, r17
	tst r16
	brne use_default_mapping
	rcall translate_ssbb2
	rjmp translate_common

use_default_mapping:
	rcall translate_default

	;;;;; Perform common mapping taksk
translate_common:

	;;;;;;;;;;;;;; Simulate analogic L/R buttons according to the corresponding buttons;
	;;;;;; Start with the 'L' button:
	rcall loadTxBufY
	ldd r16, y+GC_OFF_L
	clr r15	; Set L to 0x00
	tst r16
	breq do_L
	com r15	; Set L to 0xff
do_L:
	ldi r17, 8
	adiw yl, GC_OFF_L_VAL		; Point Y to Left button value in GC buffer
lp_L:
	st y+, r15
	dec r17
	brne lp_L

	;;;;;;;;;;;;;;; Now do the 'R' button. 
	rcall loadTxBufY
	ldd r16, y+GC_OFF_R		; R btn
	clr r15	; Set L to 0x00
	tst r16
	breq do_R
	com r15	; Set L to 0xff
do_R:
	ldi r17, 8
	adiw yl, GC_OFF_R_VAL		; Point Y to Right button value in GC buffer
lp_R:
	st y+, r15
	dec r17
	brne lp_R


	; Copy the first joystick byte
	ldi xl, lo8(n64_workbuf)
	ldi xh, hi8(n64_workbuf)
	adiw xl, N64_OFF_JOY_X
	rcall packByte ; build r18 from 8 bytes at X

	sbrs r7, R7_BIT_NOSCALE
	rcall scaleN64Value

	; now the first value is in r18. Convert
	; it to gamecube unsigned format
	ldi r17, 0x80
	eor r18, r17

	; write result to output buffer
	rcall loadTxBufY
	adiw yl, GC_OFF_JOY_X
	rcall unPackByte

	
	; Now copy the second joystick byte
	ldi xl, lo8(n64_workbuf)
	ldi xh, hi8(n64_workbuf)
	adiw xl, N64_OFF_JOY_Y
	rcall packByte ; build r18 from 8 bytes at X

	sbrs r7, R7_BIT_NOSCALE
	rcall scaleN64Value

	; now the first value is in r18. Convert it
	; to gamecube unsigned format
	ldi r17, 0x80
	eor r18, r17

	; write result to output buffer
	rcall loadTxBufY
	adiw yl, GC_OFF_JOY_Y
	rcall unPackByte

	com r6 ; swap current buffer
	ret

#include "mapping_default.S"
#include "alt_ssbm.S"
#include "alt_ssbm2.S"
#include "alt_ssbb.S"
#include "alt_ssbb2.S"

	/* Subroutine to construct a 8 bit value from 8 1bytes locations. 
	 *
	 * Arguments:
	 *   X register: Most significant bit (byte) 
     *
     * Uses:
	 *  r17, r16, r18 (result), changes X pointer
	 */
packByte:
	clr r18
	ldi r17, 0x80
packByteLp:
	ld r16, x+
	tst r16
	breq packByteWas0
	or r18, r17
packByteWas0:
	lsr r17
	brne packByteLp
	ret

	/* Subroutine to expand a byte in a register to one byte per bit 
	 *
	 * Arguments:
	 *     Y register: Destination. Most significant bit first.
	 *     r18: Input byte
	 * 
	 * Uses: r17, r15, changes Y pointer
 	 */
unPackByte:
	ldi r17, 0x80
unPackByteLp:
	; mask current bit and write result to output address
	mov r15, r17
	and r15, r18
	st y+, r15
	lsr r17
	brne unPackByteLp
	ret

	/*************************************************
	* Subroutine which scales the value in r18
	* to correct the reduced range of N64 controllers.
	*
	* Uses r16, r17, changes r18 (in/out register)
	*
	**************************************************/
	; test. Multiply by 1.5 or 1.375 with saturation
scaleN64Value:
	sbrs r7, R7_BIT_SCALE_1_5		; if bit set, use 1.5
	rjmp one_375

	; 1.5 version
	mov r16, r18
	asr r16
	add r16, r18
	rjmp saturate

one_375:
	; 1.375 version
	mov r16, r18
	asr r16 
	asr r16	
	; r16 now 1/4 of r18
	mov r17, r16
	asr r17
	; r17 now 1/8 of r18

	; Add everything up
	add r16, r18
	add r16, r17

saturate:
	; check if sign changed
	tst r16
	brmi minus
	tst r18
	brmi satpos
	rjmp nosat1
minus:
	tst r18
	brpl satneg
	rjmp nosat1
satneg:
	ldi r16, 0x7f
	mov r18, r16
	rjmp sat_done
satpos:
	ldi r16, 0x80
	mov r18, r16
	rjmp sat_done
nosat1:
	mov r18, r16
sat_done:




	/*************************************************************
	 * Subroutine which busy-loops for 6 microseconds (at 16 mhz)
	 *************************************************************/
delay_6us:
	/* At 16 Mhz, 1us is 16 cycle so 6 us is 96 cycles... */
	ldi r16, 30
delay_6us_loop:
	dec r16				; 1
	brne delay_6us_loop	; 2 when taken
	ret

	
	/*************************************************************
	 * Subroutine which busy-loops for 16 milliseconds (at 16 mhz)
	 * (it is in fact tweaked for a 59.94hz update rate
	 *************************************************************/
delay_16ms: 
	push r17
	ldi r17, 103
delay_16ms_outer_lp:
	ldi r16, 0xff
delay_16ms_lp:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	dec r16
	brne delay_16ms_lp

	nop
	nop
	nop
	nop

	dec r17
	brne delay_16ms_outer_lp
delay_16ms_done:
	pop r17
	ret
	
	/********************************************
		Uses r19, Z
	*/
build_id_packet:
	/* My controller replies with this:

		  [   Device ID   ] [status] 
		  00001001 00000000 00100000 1
		      ^  ^                   ^
			  |  |                   +--- Stop bit
			  |  +-- 0: non standard controller, 1: Dolphin standard controller
			  +----- 0: N64, 1: Dolphin
	*/
	ldi r19, 1
	ldi	zl, lo8(id_status)
	ldi zh, hi8(id_status)
	adiw zl, 4
	st z, r19	; Dolphin
	adiw zl, 3
	st z, r19	; Dolphin
	adiw zl, 11
	st z, r19	; Mysterious status bit
#if 0
	adiw zl, 6
	st z, r19	; Stop bit
#endif
	ret

build_origins_packet:
	; When Wii sends 0100 0001 1, my gc controller replies:
	; 0000 0000 1000 0000 1000 0011 0111 1010 
	; 1000 0001 0111 1110 0001 0110 0001 0011
	; 0000 0000 0000 0000 1
	;
	ldi r19, 1
	ldi zl, lo8(origins_buf)
	ldi zh, hi8(origins_buf)
	adiw zl, 8
	st z, r19
	adiw zl, 8
	st z, r19
	adiw zl, 6
	st z+, r19
	st z, r19
	adiw zl, 2
	st z+, r19
	st z+, r19
	st z+, r19
	st z, r19
	adiw zl, 2
	st z, r19
	adiw zl, 2
	st z, r19
	adiw zl, 7
	st z, r19
	adiw zl, 2
	st z+, r19
	st z+, r19
	st z+, r19
	st z+, r19
	st z+, r19
	st z, r19
	adiw zl, 5
	st z, r19
	adiw zl, 2
	st z+, r19
	st z, r19
	adiw zl, 5
	st z, r19
	adiw zl, 3
	st z+, r19
	st z, r19
	;;;; 16 bit of 0's? 
#if 0
	adiw zl, 17
	st z, r19 	; Stop bit
#endif
	ret

	/*********************************************
	*********************************************/
clr_precooked_buffers:
	clr r20
	
	ldi	zl, lo8(id_status)
	ldi zh, hi8(id_status)
	
	ldi r19, 24
lp3412:
	st z+, r20
	dec r19
	brne lp3412

	ldi	zl, lo8(origins_buf)
	ldi zh, hi8(origins_buf)
	ldi r19, 80
lp34128:
	st z+, r20
	dec r19
	brne lp34128

	; extra bits for 80 bit reply to alternate poll command
	ldi zl, lo8(gc_tx_buf0_extra)
	ldi zh, hi8(gc_tx_buf0_extra)
	ldi yl, lo8(gc_tx_buf1_extra)
	ldi yh, hi8(gc_tx_buf1_extra)
	ldi r19, 16 ; start by zero'ing all 16 bits
lp6343:
	st z+, r20
	st y+, r20
	dec r19
	brne lp6343

	ldi zl, lo8(gc_tx_buf0_extra)
	ldi zh, hi8(gc_tx_buf0_extra)
	ldi yl, lo8(gc_tx_buf1_extra)
	ldi yh, hi8(gc_tx_buf1_extra)

	ldi r16, 0xff ; now set 8th and 16th bits
	std z+7, r16
	std z+15, r16
	std y+7, r16
	std y+15, r16

	ret

	/**************************************************/
initReplyBuf:
	ldi	xl, lo8(gc_tx_buf1)
	ldi xh, hi8(gc_tx_buf1)
	ldi	zl, lo8(gc_tx_buf0)
	ldi zh, hi8(gc_tx_buf0)
	ldi yl, lo8(origins_buf)
	ldi yh, hi8(origins_buf)

	; Copy the origins reply.
	ldi r16, 64
cp_lp:
	ld r20, y+
	st z+, r20
	st x+, r20
	dec r16
	brne cp_lp
#if 0
	; And add the final 1 (stop bit)
	ldi r20, 1
	st z+, r20
	st x+, r20
#endif
	ret

