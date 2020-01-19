#include "RISCV_cpu.h"
#include "assembler.h"
#include "bit_tools.h"

std::vector<uint8_t>::iterator add_cmd(std::vector<uint8_t>::iterator &mem, uint32_t cmd) {

  *(mem++) = cmd & 0xFF;
  *(mem++) = (cmd >> 8) & 0xFF;
  *(mem++) = (cmd >> 16) & 0xFF;
  *(mem++) = (cmd >> 24) & 0xFF;
  return mem;
}

std::vector<uint8_t>::iterator load_assembler_sum(std::vector<uint8_t>::iterator &mem) {
  mem = add_cmd(mem, asm_addi(T0, ZERO, 0));
  mem = add_cmd(mem, asm_addi(T1, ZERO, 0));
  mem = add_cmd(mem, asm_addi(T2, ZERO, 10));
  mem = add_cmd(mem, asm_addi(T0, T0, 1));
  mem = add_cmd(mem, asm_add(T1, T1, T0));
  mem = add_cmd(mem, asm_beq(T0, T2, 8));
  mem = add_cmd(mem, asm_jal(ZERO, -16));
  mem = add_cmd(mem, asm_add(A0, T1, ZERO));
  mem = add_cmd(mem, asm_xor(RA, RA, RA));
  mem = add_cmd(mem, asm_jalr(ZERO, RA, 0));

  return mem;
}

std::vector<uint8_t>::iterator load_assembler_sort(std::vector<uint8_t>::iterator &mem) {
  // A1 is n and A3 points to A[0]
  // A4 is i, A5 is j, a6 is x
  mem = add_cmd(mem, asm_addi(A3, A0, 4)); // 0
  mem = add_cmd(mem, asm_addi(A4, X0, 1)); // 4
  // Outer Loop
  mem = add_cmd(mem, asm_bltu(A4, A1, +8)); // 8
  // Exit Outer Loop
  mem = add_cmd(mem, asm_jalr(X0, X1, 0)); // 0c
  // Continue Outer Loop
  mem = add_cmd(mem, asm_lw(A6, A3, 0));   // 10
  mem = add_cmd(mem, asm_addi(A2, A3, 0)); // 14
  mem = add_cmd(mem, asm_addi(A5, A4, 0)); // 18
  // Inner Loop
  mem = add_cmd(mem, asm_lw(A7, A2, -4));  // 1c
  mem = add_cmd(mem, asm_bge(A6, A7, +20));  // 20
  // sw rs2, offset(rs1) <= Note the rs1 / rs2 order.
  mem = add_cmd(mem, asm_sw(A2, A7, 0));  // 24
  mem = add_cmd(mem, asm_addi(A5, A5, -1)); // 28
  mem = add_cmd(mem, asm_addi(A2, A2, -4));  // 2c
  mem = add_cmd(mem, asm_bne(A5, X0, -20));  // 30
  // Exit Inner Loop
  mem = add_cmd(mem, asm_slli(A5, A5, 2));  // 34
  mem = add_cmd(mem, asm_add(A5, A0, A5));  // 38
  mem = add_cmd(mem, asm_sw(A5, A6, 0));  // 3c
  mem = add_cmd(mem, asm_addi(A4, A4, 1));  // 40
  mem = add_cmd(mem, asm_addi(A3, A3, 4));  // 44
  mem = add_cmd(mem, asm_jal(X0, -64));  // 48
  return mem;
}
