#pragma once

namespace std {
  void __throw_length_error(char const*) { while(true); }
  void __throw_logic_error(char const*) { while(true); }
}
#include <string>
