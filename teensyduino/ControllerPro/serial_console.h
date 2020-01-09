#pragma once
#include "console.h"

class SerialConsole : public Console {
 public:
  SerialConsole();
  virtual void cls();

 private:
  void out_impl();
};

// Shim singleton
extern SerialConsole console;

