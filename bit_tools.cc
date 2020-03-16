#include "bit_tools.h"

constexpr uint64_t BitMask(int width) {
  uint64_t mask = 0;
  for (int i = 0; i < width; i++) {
    mask = (mask << 1) | 1;
  }
  return mask;
}


