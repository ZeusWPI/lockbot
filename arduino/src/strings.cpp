#define STRINGS_CPP
#include "strings.hpp"

#include <Arduino.h>

size_t strlen(FlashString s) { //
  return strlen_P(reinterpret_cast<PGM_P>(s));
}

int strcmp(const char *s1, FlashString s2) {
  return strcmp_P(s1, reinterpret_cast<PGM_P>(s2));
}

int snprintf(char *s, size_t n, FlashString fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vsnprintf_P(s, n, reinterpret_cast<PGM_P>(fmt), ap);
  va_end(ap);
  return ret;
}
