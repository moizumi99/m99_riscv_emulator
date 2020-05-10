#ifndef LOAD_ASSEMBLER_H
#define LOAD_ASSEMBLER_H

#include <cstdint>
#include "memory_wrapper.h"
#include <vector>

using namespace RISCV_EMULATOR;

namespace CPU_TEST {

MemorWrapperIterator LoadAssemblerSum(MemorWrapperIterator &mem);

MemorWrapperIterator LoadAssemblerSort(MemorWrapperIterator &mem);

} // namespace CPU_TEST {

#endif // CPU_TEST