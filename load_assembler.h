#ifndef LOAD_ASSEMBLER_H
#define LOAD_ASSEMBLER_H

#include <cstdint>
#include "memory_wrapper.h"
#include <vector>

memory_wrapper_iterator load_assembler_sum(memory_wrapper_iterator &mem);
memory_wrapper_iterator load_assembler_sort(memory_wrapper_iterator &mem);

#endif // LOAD_ASSEMBLER_H