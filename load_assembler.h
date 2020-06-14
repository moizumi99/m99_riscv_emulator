#ifndef LOAD_ASSEMBLER_H
#define LOAD_ASSEMBLER_H

#include <cstdint>
#include "memory_wrapper.h"
#include <vector>

using namespace RISCV_EMULATOR;

namespace CPU_TEST {

uint64_t LoadAssemblerSum(MemoryWrapper &mem, uint64_t address);

uint64_t LoadAssemblerSort(MemoryWrapper &mem, uint64_t address);

} // namespace CPU_TEST {

#endif // CPU_TEST