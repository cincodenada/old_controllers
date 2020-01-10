#include "console.h"

#include "stdio.h"

void Console::log(int level, const char* format, ...) {
  va_list args;
  va_start(args, format);
  build_message(level, format, args);
  va_end(args);
}
void Console::log(const char* format, ...) {
  va_list args;
  va_start(args, format);
  build_message(DEBUG, format, args);
  va_end(args);
}

void Console::build_message(int level, const char* format, va_list args) {
  if(level > log_level) { return; }
  if(!enabled) { return; }

  int cur_len, i;
  cur_len = vsnprintf(msg, MSG_LEN, format, args);
  if(cur_len > max_len) { max_len = cur_len; }
  for(i=cur_len; i < max_len && i < MSG_LEN; i++) {
    msg[i] = ' ';
  }
  msg[i] = '\0';

  log_impl();
}
