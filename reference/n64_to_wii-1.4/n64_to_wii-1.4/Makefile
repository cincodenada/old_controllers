CC=avr-gcc
AS=$(CC)
LD=$(CC)

CPU=atmega8
UISP=uisp -dprog=stk500 -dpart=atmega8 -dserial=/dev/avr
CFLAGS=-Wall -mmcu=$(CPU)
LDFLAGS=-mmcu=$(CPU) -Wl,-Map=n64_to_wii.map
HEXFILE=n64_to_wii.hex

OBJS=main.o n64.o

all: $(HEXFILE)

clean:
	rm -f n64_to_wii.elf n64_to_wii.hex n64_to_wii.map $(OBJS)

n64_to_wii.elf: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o n64_to_wii.elf

n64_to_wii.hex: n64_to_wii.elf
	avr-objcopy -j .data -j .text -O ihex n64_to_wii.elf n64_to_wii.hex
	avr-size n64_to_wii.elf

fuse:
	$(UISP) --wr_fuse_h=0xc9 --wr_fuse_l=0x9f

flash: $(HEXFILE)
	$(UISP) --erase --upload --verify if=$(HEXFILE)

fuse_usb:
	sudo avrdude -p m8 -P usb -c avrispmkII -Uhfuse:w:0xc9:m -Ulfuse:w:0x9f:m

flash_usb: $(HEXFILE)
	sudo avrdude -p m8 -P usb -c avrispmkII -Uflash:w:$(HEXFILE) -B 1.0
	

%.o: %.S mapping_default.S alt_ssbm.S alt_ssbm2.S
	$(CC) $(CFLAGS) -c $<
