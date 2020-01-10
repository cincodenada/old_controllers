#pragma once
#include <stdio.h>
#include "../../console.h"

class SerialConsole : public Console {
 public:
  virtual void cls() {};

 private:
  void log_impl() {
    printf("%s\n", msg);
  };
};

// Shim singleton
extern SerialConsole console;
