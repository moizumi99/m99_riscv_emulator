#ifndef BIT_TOOLS_H
#define BIT_TOOLS_H

#include <stdint.h>

uint32_t bitshift(uint32_t val, int width, int offset, int distpos);
uint32_t bitcrop(uint32_t val, int width, int offset);

#endif // BIT_TOOLS_H