//
// Created by moiz on 5/16/20.
//

#ifndef ASSEMBLER_TEST_DISASSEMBLER_H
#define ASSEMBLER_TEST_DISASSEMBLER_H

#include "RISCV_cpu.h"

namespace RISCV_EMULATOR {

std::string Disassemble(uint32_t ir, int mxl = 1);

}

#endif //ASSEMBLER_TEST_DISASSEMBLER_H
