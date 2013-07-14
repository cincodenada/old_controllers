#include <avr/io.h>

.text
.global n64_sendData
.extern n64_workbuf

#define N64_DATA_BIT	3
#define N64_DATA_MASK	(1<<N64_DATA_BIT)
#define N64_IO_DDR 		_SFR_IO_ADDR(DDRC)
#define N64_IO_PIN		_SFR_IO_ADDR(PINC)

#define DATA_FALL_TIMEOUT	0x15
#define DATA_RAISE_TIMEOUT	0x15 

#define DEBUG_PORTD_BIT 0x00	/* In PORTD */
#define IO_PORTD _SFR_IO_ADDR(PORTD)

#define yl	r28
#define yh	r29

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
	* Number of bits to send is passed in r15
	* Y must point to first byte of data. Every byte
	* represents one bit (programming laziness..). A logic
	* 1 is sent when a byte is non-zero.
	* Used registers: r16, r17
	************************************************/
n64_sendData:

lp_sendGCData_bits:
	ld r16, y+
	tst r16
	brne send1
	nop
send0:
	sbi N64_IO_DDR, N64_DATA_BIT	; Pull bus to 0 
	
	ldi r17, 15
lp_send0_3us:
	dec r17
	brne lp_send0_3us
	nop

	cbi N64_IO_DDR, N64_DATA_BIT	; Release bus to 1

	ldi r17, 2
lp_send0_1us:
	dec r17
	brne lp_send0_1us

	dec r15
	brne lp_sendGCData_bits

	rjmp readResponse
;	ret

send1:
	sbi N64_IO_DDR, N64_DATA_BIT	; Pull bus to 0

	ldi r17, 4
lp_send1_1us:
	dec r17
	brne lp_send1_1us
	nop
	nop
	
	cbi N64_IO_DDR, N64_DATA_BIT	; Release bus to 1

	ldi r17, 12
lp_send1_3us:
	dec r17
	brne lp_send1_3us
	nop
	nop

	dec r15
	brne lp_sendGCData_bits
	
	; rjmp readResponse fallthrough to read response

	/********************************************************/
readResponse:
	; set error flag true (and clear it on success)
	clr r21
	com r21

	ldi	yl, lo8(n64_workbuf)
	ldi yh, hi8(n64_workbuf)
	ldi r18, 33	; 32 bit reply + stop

readNextBit:
	ldi r16, DATA_FALL_TIMEOUT	; setup a timeout
waitFall:				
	dec r16						; 1
	breq timeout				; 1
	in r17, N64_IO_PIN				; 1 : read the input port
	andi r17, N64_DATA_MASK		; 1 : isolate the input bit
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
	in r17, N64_IO_PIN					; read the input port
	cbi IO_PORTD, DEBUG_PORTD_BIT	; DEBUG
	andi r17, N64_DATA_MASK			; isolate the data bit
	st y+, r17						; store the value

	dec r18							; One less bit to receive...
	breq gotCommand

	ldi r16, DATA_RAISE_TIMEOUT
waitHigh:
	dec r16
	breq timeout
	in r17, N64_IO_PIN
	andi r17, N64_DATA_MASK
	breq waitHigh					; Still high? Keep looping

	rjmp readNextBit

gotCommand:
	; return with 0 in r15 on success
	clr r15
	clr r21 ; error flag. Same as r15 but used elsewhere.
	ret

align_delay:
	ldi r16, 3
lp634535:
	dec r16
	brne lp634535
	ret

timeout:
	; return with 1 in r15 if error
	clr r15
	inc r15
	
	; error flag.
	clr r21
	com r21
	
	ret


