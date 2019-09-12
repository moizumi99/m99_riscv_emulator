#ifndef BIT_TOOLS_H
#define BIT_TOOLS_H

#include <cstdint>

uint32_t bitshift(uint32_t val, int width, int offset, int distpos);
uint32_t bitcrop(uint32_t val, int width, int offset);
uint32_t load_wd(uint8_t *address);
void store_wd(uint8_t *address, uint32_t data);

#endif // BIT_TOOLS_H