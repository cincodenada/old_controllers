#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#define 3V_DIR DDRD
#define 3V_PORT PORTD
#define 3V_IN PIND
#define 3V_SHIFT 0
#define 5V_DIR DDRB
#define 5V_PORT PORTB
#define 5V_IN PINB
#define 5V_SHIFT 0
//+5V = SNES, GND = NES
#define SNES_DIR DDRF
#define SNES_PORT PORTF
#define SNES_IN PINF
#define SNES_SHIFT 4
//D7 = clock, D6 = latch
#define CLOCK_DIR DDRD
#define CLOCK_PORT PORTD
#define CLOCK_IN PIND
#define CLOCK_MASK 0x80
#define LATCH_DIR DDRD
#define LATCH_PORT PORTD
#define LATCH_IN PIND
#define LATCH_MASK 0x40

//For now, just the ports I have populated...
#define IO_MASK 0x0F

//C6/7 = 9/10
#define PIN_TRIGGER 10

#endif /* PIN_CONFIG_H */
