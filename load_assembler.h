#ifndef LOAD_ASSEMBLER_H
#define LOAD_ASSEMBLER_H

#include <cstdint>
#include <vector>

std::vector<uint8_t>::iterator load_assembler_sum(std::vector<uint8_t>::iterator &mem);
std::vector<uint8_t>::iterator load_assembler_sort(std::vector<uint8_t>::iterator &mem);

#endif // LOAD_ASSEMBLER_H