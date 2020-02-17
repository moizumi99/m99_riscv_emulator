#ifndef BIT_TOOLS_H
#define BIT_TOOLS_H

#include <cstdint>
#include <vector>
#include "memory_wrapper.h"

uint32_t bitshift(uint32_t val, int width, int offset, int distpos);
uint32_t bitcrop(uint32_t val, int width, int offset);

int32_t sext(uint32_t value, int width);

#endif // BIT_TOOLS_H