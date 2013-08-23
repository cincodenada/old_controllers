Old Controllers!
================

I wanted to play some N64 games on my computer, but wanted to use real controllers.  Solution?  
Well, I'm an engineer, so of course it was take out my [Teensy](http://www.pjrc.com/teensy/) and get coding.

Currently, this code will let you plug up to four N64 controllers into the Teensy and use them as USB Joysticks.
You will need my extensions to the Teensyduino hardware definitions, which will be a submodule once I figure out
how to update GitHub's reference, but for now is at [cincodenada/teensyduino-multijoy](https://github.com/cincodenada/teensyduino-multijoy).
These simply add a new hardware option to allow for two joystics.  Perhaps I'll make a branch that only supports
two controllers and can use the default Joystick to make things easier, that would be pretty easy.

Details: using the [pin diagram](https://github.com/cincodenada/old_controllers/blob/master/reference/N64%20Controller%20Protocol_files/n64pins.png),
connect the data pins (middle) to pins D2-D5 of the Teensy, connect ground to ground and VCC to a **3.3V source**

I'll repeat: you must **connect it to +3.3V**, I just used some resistors with some diodes that I had laying around,
but ***don't*** connect the controllers to 5V, because bad things may happen.

The first two controllers (D2/3) will be their own joysticks, the second two will take over the "upper half" of
the first joysticks (axes Z and Z-rot, buttons 16-32).  For some games this may cause problems, but most emulators
should be able to map them arbitrarily.

Since my Teensy was previously busy converting NES controllers to USB Keypresses, I'm endeavoring to expand this
to allow for NES and SNES controllers as well.  The multi_controller branch has my endeavors in that regard.

And in fact, I'm building a custom daughterboard that will have plugs and everything, and be smart and auto-switch
between the controllers so you can plug whatever combination you want into it.  
The board files are over at [https://github.com/cincodenada/old_controllers-pcb](cincodenada/old_controllers-pcb) if you like that kind of thing.
