#include "util.hpp"
#include "gl.h"
#include "string.hpp"

float randomLike(const unsigned int index) {
    unsigned int b = index ^ (index * 11) ^ (index / 17) ^ (index >> 16) ^ (index * 1877) ^ (index * 8332) ^ (index * 173);
    b = b ^ (b << 8) ^ (b * 23);
    b >>= 3;
    return (float)(b & 0xffff) / 0x10000;
}

void ERROR(const String &error) {
  glDone();
  printf(error.c_str());
  printf("\n");
  exit(0);
}
