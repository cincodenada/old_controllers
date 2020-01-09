#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdarg.h>
#include <stdint.h>

#define ERROR 1
#define WARN 2
#define INFO 3
#define DEBUG 4
#define TRACE 5

#define MSG_LEN 70
#define NUM_BITS 8
#define LOG_LEVEL TRACE

// These are junk drawer, TBD
extern char binstr[];
void printBin(char* dest, char input, unsigned char num_bits = NUM_BITS);
void blink_binary(int num, uint8_t bits);

class Console {
 public:
  virtual void cls() = 0;

  void out(int level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    out_impl(level, format, args);
    va_end(args);
  }
  void out(const char* format, ...) {
    va_list args;
    va_start(args, format);
    out_impl(DEBUG, format, args);
    va_end(args);
  }


  void enable(bool enabled) {
    this->enabled = enabled;
  };

 protected:
  virtual void out_impl(int level, const char* format, va_list args) = 0;

  bool enabled = true;
  int max_len = 30;

  char msg[MSG_LEN];
};

#endif /* CONSOLE_H */
