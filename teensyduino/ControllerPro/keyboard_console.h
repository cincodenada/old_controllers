#pragma once
#include "console.h"

enum class KeyboardMode {
  STANDARD,
  VIM,
};

class KeyboardConsole : public Console {
 public:
  virtual void cls();

  void set_mode(KeyboardMode mode) { this->mode = mode; };

 private:
  void out_impl();

  KeyboardMode mode;
};

// Shim singleton
extern KeyboardConsole console;

