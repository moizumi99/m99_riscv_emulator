#ifndef LOAD_ASSEMBLER_H
#define LOAD_ASSEMBLER_H

#include <cstdint>
#include "memory_wrapper.h"
#include <vector>

MemorWrapperIterator LoadAssemblerSum(MemorWrapperIterator &mem);
MemorWrapperIterator LoadAssemblerSort(MemorWrapperIterator &mem);

#endif // LOAD_ASSEMBLER_H