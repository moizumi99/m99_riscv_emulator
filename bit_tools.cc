#include "bit_tools.h"

uint32_t mask[] = {0x0,        0x01,       0x03,       0x07,      0x0F,
                   0x01F,      0x03F,      0x07F,      0x0FF,     0x01FF,
                   0x03FF,     0x07FF,     0x0FFF,     0x01FFF,   0x03FFF,
                   0x07FFF,    0x0FFFF,    0x01FFFF,   0x03FFFF,  0x07FFFF,
                   0x0FFFFF,   0x01FFFFF,  0x03FFFFF,  0x07FFFFF, 0x0FFFFFF,
                   0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF};

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

uint32_t load_wd(uint8_t *address) {
  return *address | (*(address + 1) << 8) | (*(address + 2) << 16) | (*(address + 3) << 24);
}

void store_wd(uint8_t *address, uint32_t data) {
  *address = data & 0xFF;
  *(address + 1) = (data >> 8) & 0xFF;
  *(address + 2) = (data >> 16) & 0xFF;
  *(address + 3) = (data >> 24) & 0xFF;
}
