#pragma once

#include <string.h>
#include <vector>

class SimpleString : public std::vector<char> {
 public:
  using Base = std::vector<char>;
  // Copies string into vector
  SimpleString(const char* orig) : Base() {
    while(orig != '\0') { push_back(*orig); }
  }

  bool operator==(const char* other) {
    if(strncmp(other, data(), size()) == 0) {
      return other[size()] == '\0';
    }
    return false;
  }
};