#include "load_assembler.h"
#include "RISCV_Emulator.h"
#include "RISCV_cpu.h"
#include "assembler.h"
#include "bit_tools.h"
#include "instruction_encdec.h"

using namespace std;

uint8_t *add_cmd(uint8_t *mem, uint32_t cmd) {
  *(mem++) = cmd & 0xFF;
  *(mem++) = (cmd >> 8) & 0xFF;
  *(mem++) = (cmd >> 16) & 0xFF;
  *(mem++) = (cmd >> 24) & 0xFF;
  return mem;
}

uint8_t *load_assembler_sum(uint8_t *mem) {
  mem = add_cmd(mem, asm_addi(T0, ZERO, 0));
  mem = add_cmd(mem, asm_addi(T1, ZERO, 0));
  mem = add_cmd(mem, asm_addi(T2, ZERO, 10));
  mem = add_cmd(mem, asm_addi(T0, T0, 1));
  mem = add_cmd(mem, asm_add(T1, T1, T0));
  mem = add_cmd(mem, asm_beq(T0, T2, 8));
  mem = add_cmd(mem, asm_jal(ZERO, -16));
  mem = add_cmd(mem, asm_add(A0, T1, ZERO));
  mem = add_cmd(mem, asm_jalr(ZERO, RA, 0));

  return mem;
}
