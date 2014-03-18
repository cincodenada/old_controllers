int cycle = 31;
int flip;
int numsticks;

void setup() {
    Serial.begin(9600);

    Serial.println("Initiating controllers");

    MultiJoystick.setJoyNum(0);
    MultiJoystick.useManualSend(true); 
    numsticks = MultiJoystick.num_joys();
}

void loop() {
    flip = cycle % 2; 
    for(short int cnum=0; cnum < numsticks; cnum++) {
        Serial.println(cnum);
        MultiJoystick.setJoyNum(cnum);
        
        //Cycle through all the buttons, whee!
        MultiJoystick.button((cycle+31)%32+1,0);
        MultiJoystick.button(cycle+1,1);
        //Also the stick, because why not?
        MultiJoystick.hat(cycle*11);

        //Flash the current button
        MultiJoystick.button(cnum+1,flip);

        //Flash the axes to indicate how many joysticks we have
        for(int i=0;i<numsticks;i++) {
          MultiJoystick.axis(i+1,1023*flip);
        }
        MultiJoystick.send_now();
    }

    cycle = (cycle + 1) % 32;
    delay(100);
}
