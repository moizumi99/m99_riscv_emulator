#ifndef RISCV_CPU_H
#define RISCV_CPU_H
#include <stdint.h>
#include "bit_tools.h"

int run_cpu(uint32_t *rom);

enum Registers {
  ZERO = 0,
  RA = 1,
  SP = 2,
  GP = 3,
  TP = 4,
  T0 = 5,
  T1 = 6,
  T2 = 7,
  FP = 8,
  S0 = 8,
  S1 = 9,
  A0 = 10,
  A1 = 11,
  A2 = 12,
  A3 = 13,
  A4 = 14,
  A5 = 15,
  A6 = 16,
  A7 = 17,
  S2 = 18,
  S3 = 19,
  S4 = 20,
  S5 = 21,
  S6 = 22,
  S7 = 23,
  S8 = 24,
  S9 = 25,
  S10 = 26,
  S11 = 27,
  T3 = 28,
  T4 = 29,
  T5 = 30,
  T6 = 31
};

enum op_label {
  OPCODE_ADD = 0b0110011,
  OPCODE_ADDI = 0b0010011,
  OPCODE_B = 0b1100011,
  OPCODE_LD = 0b0000011,
  OPCODE_J = 0b1101111,
  OPCODE_S = 0b0100011,
  OPCODE_JALR = 0b1100111,
};

enum op_funct {
  FUNC_ADD = 0b0000000,
  FUNC_ADDI = 0b0000000,
  FUNC_SUB = 0b0100000,
  FUNC_OR = 0b0000000,
  FUNC_SLLI = 0b000000,
  FUNC_SRLI = 0b000000,
  FUNC_SRAI = 0b010000,
  FUNC_BEQ = 0b0000000,
  FUNC_JAL = 0b0000000,
  FUNC_LD = 0b0000000,
  FUNC_SW = 0b0000000,
  FUNC_JALR = 0b0000000
};

enum op_funct3 {
  FUNC3_ADD = 0b000,
  FUNC3_ADDI = 0b000,
  FUNC3_SUB = 0b000,
  FUNC3_OR = 0b110,
  FUNC3_SLLI = 0b001,
  FUNC3_SRLI = 0b101,
  FUNC3_SRAI = 0b101,
  FUNC3_BEQ = 0b000,
  FUNC3_LD = 0b011,
  FUNC3_SW = 0b010,
  FUNC3_JALR = 0b000,
};

enum instruction {
  INST_ERROR = 0,
  INST_ADD = 1,
  INST_SUB = 2,
  INST_ADDI = 3,
  INST_BEQ = 4,
  INST_JAL = 5,
  INST_JALR = 6,
};


#endif // RISCV_CPU_H