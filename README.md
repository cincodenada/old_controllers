Old Controllers!
================

I wanted to play some N64 games on my computer, but wanted to use real controllers.  Solution?  
Well, I'm an engineer, so of course it was take out my [Teensy](http://www.pjrc.com/teensy/) and get coding. This code now requires a Teensy 3.2 or higher. It may work on a Teensy LC, but that is untested.

This code will let you plug up to four N64, NES, or SNES controllers into the Teensy and use them as USB Joysticks.
You will need my extensions to the Teensyduino hardware definitions, which is here as a submodule in teensyduino/cores, with the repo at [my fork of the Teensyduino cores library](https://github.com/cincodenada/cores/). These add new USB device profiles to support more than one joystick. Currently there are two different methods used, one called MultiJoystick and one called Gamepad. The former includes a serial port for debugging, but has been less reliable in my testing.

This code is now part of a larger project involving a dedicated breakout board that is documented [over at Hackster.io](https://www.hackster.io/cincodenada/controller-pro-9f80e8), but it should still work standalone, although I haven't tested without the breakout board for a while!

That said, details for hooking up on your own, last I checked: using the [pin diagram](https://github.com/cincodenada/old_controllers/blob/master/reference/N64%20Controller%20Protocol_files/n64pins.png),
connect the data pins (middle) to the `FAST_PINS` (`FP0..3`) configured in `pin_config.h` (note, these are Arduino-compatible pin numbers), connect ground to ground and VCC to 3.3V. ***Don't*** connect the controllers to 5V, because the N64 controllers run at 3.3V and using 5V may damage them. This shouldn't be a problem, because Teensy 3 is native at 3.3V, but if you're trying to use a different Arduino or what have you, take note!

SNES/NES use three pins plus VCC/GND. They natively run at 5V, but they're just CMOS chips inside, so they can run at 3.3V within spec just fine. Referring to the [SNES/NES pin diagram](https://github.com/cincodenada/old_controllers/blob/master/reference/NesSnesPinout_r.jpg), connect the appropriate pin of each controller to `LATCH_PIN` and `CLOCK_PIN` (these are shared across controllres), and then data lines for each controller to the `SLOW_PINS` (`SP0..3`). The data line for each should also be pulled low with a pulldown resistor of something like 20-50kΩ.

Due to the limited USB resources of the Teensy two, this code originally mapped two phyiscal controllers onto each virtual controller to achieve the 3rd and 4th controller. Teensy 3 has more endpoints available, so this is no longer necessary there are some traces of this remaining in the code, but they will probably be removed as I tidy things up.

There is also a custom breakout board for the Teensy that has plugs and everything, and is very clever and auto-switches
between the controllers so you can plug whatever combination you want into it. The board files are over at [cincodenada/old_controllers-pcb](https://github.com/cincodenada/old_controllers-pcb), or in the `pcb` submodule here if you like that kind of thing.
