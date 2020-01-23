#ifndef BIT_TOOLS_H
#define BIT_TOOLS_H

#include <cstdint>
#include <vector>
#include "memory_wrapper.h"

uint32_t bitshift(uint32_t val, int width, int offset, int distpos);
uint32_t bitcrop(uint32_t val, int width, int offset);
uint32_t load_wd(const memory_wrapper_iterator &&address);
void store_wd(memory_wrapper_iterator &&address, uint32_t data, int width = 32);
int32_t sext(uint32_t value, int width);

#endif // BIT_TOOLS_H