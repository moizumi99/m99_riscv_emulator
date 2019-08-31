#include "load_assembler.h"
#include "RISCV_Emulator.h"
#include "RISCV_cpu.h"

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

uint32_t mask[] = {0x01,       0x03,       0x07,      0x0F,      0x01F,
                   0x03F,      0x07F,      0x0FF,     0x01FF,    0x03FF,
                   0x07FF,     0x0FFF,     0x01FFF,   0x03FFF,   0x07FFF,
                   0x0FFFF,    0x01FFFF,   0x03FFFF,  0x07FFFF,  0x0FFFFF,
                   0x01FFFFF,  0x03FFFFF,  0x07FFFFF, 0x0FFFFFF, 0x01FFFFFF,
                   0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF};

uint32_t bitshift(uint32_t val, int width, int offset, int distpos) {
  val >>= offset;
  val &= mask[width];
  val <<= distpos;
  return val;
}

uint32_t asm_add(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint32_t cmd = LABEL_R | (FUNC_ADD << 25) | (FUNC3_ADD << 12);
  return cmd | bitshift(rd, 5, 0, 7) | bitshift(rs1, 5, 0, 15) | bitshift(rs2, 5, 0, 20);
}

uint32_t asm_addi(uint32_t rd, uint32_t rs1, uint32_t imm) {
  uint32_t cmd = LABEL_I | (FUNC3_ADDI << 12);
  return cmd | (rd & 0x1F << 7) | (rs1 & 0x1F << 15) | (imm & 0xFFF << 20);
}

uint32_t asm_sub(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint32_t cmd = LABEL_R | (FUNC_SUB << 25) | (FUNC3_SUB << 12);
  return cmd | (rd & 0x1F << 7) | (rs1 & 0x1F << 15) | (rs2 & 0x1F << 20);
}

uint32_t asm_beq(uint32_t rs1, uint32_t rs2, uint32_t offset) {
  uint32_t cmd = LABEL_B | (FUNC3_BEQ << 12);
  cmd |= (rs2 & 0x1F << 20) | (rs1 & 0x1F << 15);
  cmd |= (offset & 0x1000 << 19) | (offset & 0b011111100000 << 20) |
         (offset & 0b011110 << 7) | (offset & 0b0100000000000 >> 4);
  return cmd;
}

uint32_t asm_jal(uint32_t rd, uint32_t offset) {
  uint32_t cmd = LABEL_J;
  cmd |= (rd & 0x1F << 7);
  cmd |= (offset & 0x100000 << 11) | (offset & 0b011111111110 << 20) |
         (offset & 0x400 << 9) | (offset & 0b011111111000000000000);
  return cmd;
}

uint32_t asm_ld(uint32_t rd, uint32_t rs1, uint32_t offset) {
  uint32_t cmd = LABEL_I | (FUNC3_LD << 12);
  cmd |=
      (rd & 0x1F << 7) | (rs1 & 0x1F << 15) | (offset & 0b0111111111111 << 20);
  return cmd;
}

uint32_t asm_sw(uint32_t rs1, uint32_t rs2, uint32_t offset) {
  uint32_t cmd = LABEL_I | (FUNC3_SW << 12);
  cmd |= (rs2 & 0x1F << 20) | (rs1 & 0x1F << 15);
  cmd |= (offset & 0b0111111100000 << 20) | (offset & 0b011111 << 7);
  return cmd;
}
