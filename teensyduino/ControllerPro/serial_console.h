#pragma once
#include "console.h"

#include <memory>

class SerialConsole : public Console {
 public:
  SerialConsole();
  virtual void cls();

  static std::shared_ptr<SerialConsole> getInstance() {
    if(!instance) {
      instance = std::make_unique<SerialConsole>();
    }
    return instance;
  }

 private:
  void out_impl(int level, const char* format, va_list args);
  static std::shared_ptr<SerialConsole> instance;
};

