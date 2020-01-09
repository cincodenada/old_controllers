#include "keyboard_console.h"
#include "Arduino.h"

//KeyboardConsole console;

void KeyboardConsole::out_impl() {
  Keyboard.println(msg);
}

void KeyboardConsole::cls() {
  switch(mode) {
    case KeyboardMode::STANDARD:
      Keyboard.press(MODIFIERKEY_CTRL);
      Keyboard.press(KEY_A);
      Keyboard.release(KEY_A);
      Keyboard.release(MODIFIERKEY_CTRL);
      Keyboard.press(KEY_BACKSPACE);
      break;
    case KeyboardMode::VIM:
      Keyboard.press(KEY_ESC);
      Keyboard.release(KEY_ESC);
      Keyboard.print("ggVGdi");
      break;
  }
}
