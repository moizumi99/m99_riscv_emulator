#ifndef RISCV_CPU_H
#define RISCV_CPU_H
#include <stdint.h>

int run_cpu(uint32_t *rom);

enum Registers {
  ZERO = 0,
  T0 = 5,
  T1 = 6,
  T2 = 7,
  T3 = 28
};

enum op_label {
  LABEL_R = 0b0110011,
  LABEL_I = 0b0010011,
  LABEL_B = 0b1100011,
  LABEL_J = 0b1101111,
  LABEL_S = 0b0100011
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
  FUNC_SW = 0b0000000
};

enum op_funct3 {
  FUNC3_ADD = 0b000,
  FUNC3_ADDI = 0b000,
  FUNC3_SUB = 0b000,
  func3_or = 0b110,
  FUNC3_SLLI = 0b001,
  FUNC3_SRLI = 0b101,
  FUNC3_SRAI = 0b101,
  FUNC3_BEQ = 0b000,
  FUNC3_JAL = 0b000,
  FUNC3_LD = 0b011,
  FUNC3_SW = 0b010
};

#endif //RISCV_CPU_H