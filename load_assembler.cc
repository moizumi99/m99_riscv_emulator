#include "load_assembler.h"
#include "RISCV_Emulator.h"
#include "RISCV_cpu.h"
#include "bit_tools.h"
#include "instruction_encdec.h"

using namespace std;

uint32_t asm_add(uint32_t, uint32_t, uint32_t);
uint32_t asm_addi(uint32_t, uint32_t, uint32_t);
uint32_t asm_sub(uint32_t, uint32_t, uint32_t);
uint32_t asm_and(uint32_t, uint32_t, uint32_t);
uint32_t asm_or(uint32_t, uint32_t, uint32_t);
uint32_t asm_slli(uint32_t, uint32_t, uint32_t);
uint32_t asm_srli(uint32_t, uint32_t, uint32_t);
uint32_t asm_srai(uint32_t, uint32_t, uint32_t);
uint32_t asm_beq(uint32_t, uint32_t, uint32_t);
uint32_t asm_jal(uint32_t, uint32_t);
uint32_t asm_ld(uint32_t, uint32_t, uint32_t);
uint32_t asm_sw(uint32_t, uint32_t, uint32_t);

void load_assembler(uint32_t *rom) {
  rom[0] = asm_addi(T0, ZERO, 0);
  rom[1] = asm_addi(T1, ZERO, 0);
  rom[2] = asm_addi(T2, ZERO, 10);
  rom[3] = asm_addi(T3, ZERO, 64);
  rom[4] = asm_addi(T0, T0, 1);
  rom[5] = asm_add(T1, T1, T0);
  rom[6] = asm_sw(T0, T3, 0);
  rom[7] = asm_beq(T0, T2, 8);
  rom[8] = asm_jal(ZERO, -16);
  rom[9] = asm_jal(ZERO, 0);
}

uint32_t asm_add(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  r_type cmd;
  cmd.funct7 = FUNC_ADD;
  cmd.opcode = OPCODE_ADD;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.rd = rd;
  cmd.funct3 = FUNC3_ADD;
  return cmd.value();
}

uint32_t asm_addi(uint32_t rd, uint32_t rs1, uint32_t imm12) {
  uint32_t cmd = LABEL_I | (FUNC3_ADDI << 12);
  return cmd | bitshift(rd, 5, 0, 7) | bitshift(rs1, 5, 0, 15) |
         bitshift(imm12, 12, 0, 20);
}

uint32_t asm_sub(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint32_t cmd = LABEL_R | (FUNC_SUB << 25) | (FUNC3_SUB << 12);
  return cmd | bitshift(rd, 5, 0, 7) | bitshift(rs1, 5, 0, 15) |
         bitshift(rs2, 5, 0, 20);
}

uint32_t asm_beq(uint32_t rs1, uint32_t rs2, uint32_t offset13) {
  uint32_t cmd = LABEL_B | (FUNC3_BEQ << 12);
  cmd |= bitshift(rs2, 5, 0, 20) | bitshift(rs1, 5, 0, 15);
  cmd |= bitshift(offset13, 1, 12, 31) | bitshift(offset13, 6, 5, 25);
  cmd |= bitshift(offset13, 4, 1, 8) | bitshift(offset13, 1, 11, 7);
  return cmd;
}

uint32_t asm_jal(uint32_t rd, uint32_t offset21) {
  uint32_t cmd = LABEL_J;
  cmd |= bitshift(rd, 5, 0, 7);
  cmd |= bitshift(offset21, 1, 20, 31) | bitshift(offset21, 10, 1, 21);
  cmd |= bitshift(offset21, 1, 11, 20) | bitshift(offset21, 8, 12, 12);
  return cmd;
}

uint32_t asm_ld(uint32_t rd, uint32_t rs1, uint32_t offset12) {
  uint32_t cmd = OPCODE_LD | (FUNC3_LD << 12);
  cmd |= bitshift(rd, 5, 0, 7) | bitshift(rs1, 5, 0, 15) | bitshift(offset12, 12, 0, 20);
  return cmd;
}

uint32_t asm_sw(uint32_t rs1, uint32_t rs2, uint32_t offset12) {
  uint32_t cmd = LABEL_I | (FUNC3_SW << 12);
  cmd |= bitshift(rs2, 5, 0, 20) | bitshift(rs1, 5, 0, 15);
  cmd |= bitshift(offset12, 7, 5, 25) | bitshift(offset12, 5, 0, 7);
  return cmd;
}
