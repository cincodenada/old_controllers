#pragma once
#include "console.h"

class SerialConsole : public Console {
 public:
  SerialConsole();
  virtual void cls();

 private:
  void out_impl(int level, const char* format, va_list args);
};

// Shim singleton
extern SerialConsole console;

