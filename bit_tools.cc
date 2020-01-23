#include <stdexcept>
#include <vector>
#include "bit_tools.h"
#include "memory_wrapper.h"

uint32_t mask[] = {0x0, 0x01, 0x03, 0x07, 0x0F,
                   0x01F, 0x03F, 0x07F, 0x0FF, 0x01FF,
                   0x03FF, 0x07FF, 0x0FFF, 0x01FFF, 0x03FFF,
                   0x07FFF, 0x0FFFF, 0x01FFFF, 0x03FFFF, 0x07FFFF,
                   0x0FFFFF, 0x01FFFFF, 0x03FFFFF, 0x07FFFFF, 0x0FFFFFF,
                   0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF};

uint32_t sign_mask[] = {0xFFFFFFFE, 0xFFFFFFFC, 0xFFFFFFF8,
                        0xFFFFFFF0, 0xFFFFFFE0, 0xFFFFFFC0, 0xFFFFFF80,
                        0xFFFFFF00, 0xFFFFFE00, 0xFFFFFC00, 0xFFFFF800,
                        0xFFFFF000, 0xFFFFE000, 0xFFFFC000, 0xFFFF8000,
                        0xFFFF0000, 0xFFFE0000, 0xFFFC0000, 0xFFF80000,
                        0xFFF00000, 0xFFE00000, 0xFFC00000, 0xFF800000,
                        0xFF000000, 0xFE000000, 0xFC000000, 0xF8000000,
                        0xF0000000, 0xE0000000, 0xC0000000, 0x80000000, 0x00000000,
};

uint32_t bitshift(uint32_t val, int width, int offset, int distpos) {
  val >>= offset;
  val &= mask[width];
  val <<= distpos;
  return val;
}

uint32_t bitcrop(uint32_t val, int width, int offset) {
  val >>= offset;
  val &= mask[width];
  return val;
}

uint32_t load_wd(const memory_wrapper_iterator &&address) {
  return address[0] | (address[1] << 8) | (address[2] << 16) | (address[3] << 24);
}

void store_wd(memory_wrapper_iterator &&address, uint32_t data, int width) {
  switch(width) {
    case 32:
      address[2] = (data >> 16) & 0xFF;
      address[3] = (data >> 24) & 0xFF;
    case 16:
      address[1] = (data >> 8) & 0xFF;
    case 8:
      address[0] = data & 0xFF;
      break;
    default:
      throw std::invalid_argument("Store width is not 8, 16, or 32.");
  }
}

int32_t sext(uint32_t value, int width) {
  if (value & (1 << (width - 1))) {
    value |= sign_mask[width - 1];
  }
  return value;
}

