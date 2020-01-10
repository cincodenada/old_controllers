void truth_table() {
  for(int i=0; i < NUM_CONTROLLERS; i++) {
    pinMode(slow_pins[i], INPUT_PULLUP);
    pinMode(CLOCK_PIN, INPUT);
    digitalWrite(s_nes_pins[i], LOW);
  }
  while(true) {
    for(int latch=0; latch <=1; latch++) {
      for(int snes=0; snes <=1; snes++) {
        digitalWrite(LATCH_PIN, latch);
        for(int i=0; i<NUM_CONTROLLERS; i++) {
          digitalWrite(s_nes_pins[i], snes);
        }
        for(int i=0; i<NUM_CONTROLLERS; i++) {
          console.log("%d: %d%d%d%d",
            i,
            digitalRead(slow_pins[i]),
            latch,
            digitalRead(CLOCK_PIN),
            snes);
          delay(1);
        }
      }
    }
    delay(500);
    console.cls();
  }
}
void debug_detect() {
  while(true) {
    for(int i=0; i < NUM_CONTROLLERS; i++) {
      pinMode(slow_pins[i], INPUT);
      digitalWrite(s_nes_pins[i], LOW);
    }
    delay(1);
    for(int i=0; i < NUM_CONTROLLERS; i++) {
      pinMode(slow_pins[i], INPUT);
      digitalWrite(s_nes_pins[i], HIGH);
    }
    delay(1);
  }
}
void debug_detect2() {
  while(true) {
    pins_used = 0;
    clist[N64]->init();
    clist[SNES]->init();
    clist[NES]->init();

    for(int i=0; i < NUM_CONTROLLERS; i++) {
      unsigned char mask = 1 << i;
      char controller[] = "None";
      bool matched = false;
      for(int i=0; i < NUMCTL; i++) {
        if(clist[i]->pinmask & mask) {
          if(matched) {
            console.log("Duplicate pinmask! Also on %s", clist[i]->controller_name);
          } else {
            strncpy(controller, clist[i]->controller_name, 5);
          }
        }
      }
      console.log("Slot %d: %s", i+1, controller);
    }
    delay(100);
  }
}

