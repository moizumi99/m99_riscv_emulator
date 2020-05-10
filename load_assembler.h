#ifndef LOAD_ASSEMBLER_H
#define LOAD_ASSEMBLER_H

#include <cstdint>
#include "memory_wrapper.h"
#include <vector>

namespace RISCV_EMULATOR {

MemorWrapperIterator LoadAssemblerSum(MemorWrapperIterator &mem);

MemorWrapperIterator LoadAssemblerSort(MemorWrapperIterator &mem);

} // namespace RISCV_EMULATOR {

#endif // LOAD_ASSEMBLER_H