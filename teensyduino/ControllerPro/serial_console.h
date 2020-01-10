#pragma once
#include "console.h"

class SerialConsole : public Console {
 public:
  SerialConsole();
  virtual void cls();

 private:
  void log_impl();
};

// Shim singleton
extern SerialConsole console;

