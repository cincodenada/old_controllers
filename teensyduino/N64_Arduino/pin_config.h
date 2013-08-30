#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#define DATA_DIR DDRD
#define DATA_PORT PORTD
#define DATA_SHIFT 0
//Low = N64, High = NES/SNES
#define N64_DIR DDRB
#define N64_PORT PORTB
#define N64_SHIFT 0
//+5V = SNES, GND = NES
#define SNES_DIR DDRF
#define SNES_PORT PORTF
#define SNES_SHIFT 4
//D7 = clock, D6 = latch
#define CLOCK_DIR DDRD
#define CLOCK_PORT PORTD
#define CLOCK_MASK 0x80
#define LATCH_DIR DDRD
#define LATCH_PORT PORTD
#define LATCH_MASK 0x40

//For now, just the ports I have populated...
#define IO_MASK 0x0A

//C6/7 = 9/10
#define PIN_TRIGGER 11

#define AXIS_MAX (32767)
#define AXIS_MIN (-32768)
#define JOY_FACT ((AXIS_MAX-AXIS_MIN+1)/1024)
#define JOY_OFFSET (512);

#endif /* PIN_CONFIG_H */
