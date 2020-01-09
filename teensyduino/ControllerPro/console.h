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

class Console {
 public:
  virtual void cls() = 0;

  void out(int level, const char* format, ...);
  void out(const char* format, ...);
  void build_message(int level, const char* format, va_list args);

  void enable(bool enabled) {
    this->enabled = enabled;
  };

  void set_level(uint8_t level) {
    this->log_level = level;
  };

 protected:
  // Writes the contents of msg to the output
  virtual void out_impl() = 0;

  bool enabled = true;
  int max_len = 30;
  uint8_t log_level = INFO;

  char msg[MSG_LEN];
};

#endif /* CONSOLE_H */
