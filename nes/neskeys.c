/* Keyboard example with debug channel, for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_keyboard.html
 * Copyright (c) 2008 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usb_keyboard_debug.h"
#include "print.h"

#define LED_CONFIG	(DDRD |= (1<<6))
#define LED_ON		(PORTD &= ~(1<<6))
#define LED_OFF		(PORTD |= (1<<6))
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))
#define SLOW 1
#define NUMP 2

uint8_t key_names[2][8] = {
    {'A','B','S','S','^','v','<','>'},
    {' ',' ','e','t',' ',' ',' ',' '}
};
uint8_t key_press[NUMP][8] = {
    {KEY_X,KEY_Z,KEY_ENTER,KEY_CTRL,KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT},
    {KEY_ALT,KEY_SHIFT,KEY_Q,KEY_E,KEY_W,KEY_S,KEY_A,KEY_D}
};
uint8_t is_modifier[NUMP][8] = {
    {0,0,0,1,0,0,0,0},
    {1,1,0,0,0,0,0,0}
};

uint16_t idle_count=0;

int main(void)
{
	// set for 16 MHz clock
	CPU_PRESCALE(0);

	// See the "Using I/O Pins" page for details.
	// http://www.pjrc.com/teensy/pins.html
	DDRC = 0x06;    // 00000110 - pins 1 & 2 output, pin 3 (etc) input
	DDRF = 0x06;    // 00000110 - pins 1 & 2 output, pin 3 (etc) input
	PORTC = 0xF9;   // 11111001 pins 1 & 2 low to begin, everything else pullup
	PORTF = 0xF9;   // 11111001 pins 1 & 2 low to begin, everything else pullup

	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	usb_init();
	while (!usb_configured()) /* wait */ ;

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000);

	// Configure timer 0 to generate a timer overflow interrupt every
	// 256*1024 clock cycles, or approx 61 Hz when using 16 MHz clock
	// This demonstrates how to use interrupts to implement a simple
	// inactivity timeout.
	TCCR0A = 0x00;
	TCCR0B = 0x05;
	TIMSK0 = (1<<TOIE0);

	print("Begin keyboard example program\n");
	print("All Port C pins are inputs with pullup resistors.\n");
	while (1) {
    }
}

uint8_t poll_controller(volatile uint8_t *port, volatile uint8_t *pin) {
    uint8_t resp = 0;
    uint8_t curbit;

    //Send a 12-us pulse to the latch pin
    *port |= (1<<2);
    _delay_us(12*SLOW);
    *port &= ~(1<<2);
    //Record A
    if (!(*pin & (1<<3))) {    //Check for ground-assertion on input pin
        resp |= 1;
    }

    //Wait six us, then send 8x 12-us (50% duty cycle) pulses and record input
    curbit = 1;
    while(curbit < 8) {
        _delay_us(6*SLOW);
        *port |= (1<<1);            //Send 6ms pulse
        _delay_us(6*SLOW);
        *port &= ~(1<<1);           //Reset to ground
        if (!(*pin & (1<<3))) {    //Check for ground-assertion on input pin
            resp |= 1 << curbit;
        }
        curbit++;
    }

    return resp;
}

// This interrupt routine is run approx 61 times per second.
// A very simple inactivity timeout is implemented, where we
// will send a space character and print a message to the
// hid_listen debug message window.
ISR(TIMER0_OVF_vect)
{
    int resp[NUMP];
    resp[0] = poll_controller(&PORTC, &PINC);
    resp[1] = poll_controller(&PORTF, &PINF);

    print("Buttons pressed:\n");
    print("P1 P2 P3 P4\n");

    //Reset the USB keyboard registers
    memset(keyboard_keys, 0, 6);
    keyboard_modifier_keys = 0;

    //Initialize the counters
    int curbit, curkey, curp;
    int hadkey;
    curkey = 0;
    for(curbit = 0; curbit < 8; curbit++) {
        hadkey = 0;
        for(curp = 0; curp < NUMP; curp++) {
            //Check the current bit
            if(resp[curp] & (1 << curbit)) {
                hadkey = 1;
                pchar(key_names[0][curbit]);
                pchar(key_names[1][curbit]);
                pchar(' ');
                //Set the bits in the proper keyboard register
                if(is_modifier[curp][curbit]) {
                    keyboard_modifier_keys |= key_press[curp][curbit];
                } else {
                    if(curkey < 6) {
                        keyboard_keys[curkey] = key_press[curp][curbit];
                        curkey++;
                    } else {
                        //If we overflowed the 6 keys at once, send an error
                        pchar(8);   //Backspace
                        pchar('!');
                        pchar(7);   //Bell
                    }
                }
            } else {
                print("   ");
            }
        }
        if(hadkey) {
            pchar('\n');
        } else {
            for(int i=0; i < NUMP*3; i++) {
                pchar(8);
            }
        }
    }

    //Update the keyboard state
    usb_keyboard_send();
}


