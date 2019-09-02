#include "instruction_encdec.h"
#include "bit_tools.h"
#include <stdint.h>

uint32_t r_type::value() {
  uint32_t value = 0;
  value |= funct7 << 25 | rs2 << 20 | rs1 << 15;
  value |= funct3 << 12 | rd << 7 | opcode;
  return value;
};

void r_type::set_value(uint32_t value) {
  funct7 = bitcrop(value, 7, 25);
  rs2 = bitcrop(value, 5, 20);
  rs1 = bitcrop(value, 5, 15);
  funct3 = bitcrop(value, 3, 12);
  rd = bitcrop(value, 5, 7);
  opcode = bitcrop(value, 7, 0);
};

uint32_t i_type::value() {
  uint32_t value = 0;
  value |= imm12 << 20 | rs1 << 15;
  value |= funct3 << 12 | rd << 7 | opcode;
  return value;
};

void i_type::set_value(uint32_t value) {
  imm12 = get_imm12(value);
  rs1 = bitcrop(value, 5, 15);
  funct3 = bitcrop(value, 3, 12);
  rd = bitcrop(value, 5, 7);
  opcode = bitcrop(value, 7, 0);
};

uint32_t s_type::value() {
  uint32_t value = 0;
  uint32_t imm11_5_7 = bitcrop(imm12, 7, 5);
  uint32_t imm4_0_5 = bitcrop(imm12, 5, 0);
  value |= imm11_5_7 << 25 | rs2 << 20 | rs1 << 15;
  value |= funct3 << 12 | imm4_0_5 << 7 | opcode;
  return value;
};

void s_type::set_value(uint32_t value) {
  rs2 = bitcrop(value, 5, 20);
  rs1 = bitcrop(value, 5, 15);
  funct3 = bitcrop(value, 3, 12);
  opcode = bitcrop(value, 7, 0);
  imm12 = get_imm12(value);
};

uint32_t b_type::value() {
  uint32_t value = 0;
  uint32_t imm12_1 = bitcrop(imm13, 1, 12);
  uint32_t imm10_5_6 = bitcrop(imm13, 6, 5);
  uint32_t imm4_1_4 = bitcrop(imm13, 4, 1);
  uint32_t imm11_1 = bitcrop(imm13, 1, 11);
  value |= imm12_1 << 31 | imm10_5_6 << 25;
  value |= rs2 << 20 | rs1 << 15;
  value |= funct3 << 12 | imm4_1_4 << 8 | imm11_1 << 7 | opcode;
  return value;
};

void b_type::set_value(uint32_t value) {
  rs2 = bitcrop(value, 5, 20);
  rs1 = bitcrop(value, 5, 15);
  funct3 = bitcrop(value, 3, 12);
  opcode = bitcrop(value, 7, 0);
  imm13 = get_imm13(value);
};

uint32_t u_type::value() {
  uint32_t value = 0;
  value |= bitcrop(imm32, 24, 12) << 12;
  value |= rd << 7 | opcode;
  return value;
};

void u_type::set_value(uint32_t value) {
  imm32 = bitcrop(value, 24, 12) << 12;
  rd = bitcrop(value, 5, 7);
  opcode = bitcrop(value, 7, 0);
};

uint32_t j_type::value() {
  uint32_t value = 0;
  uint32_t imm20_1 = bitcrop(imm21, 1, 20);
  uint32_t imm10_1_10 = bitcrop(imm21, 10, 1);
  uint32_t imm11_1 = bitcrop(imm21, 1, 11);
  uint32_t imm19_12 = bitcrop(imm21, 8, 12);
  value |=
      (imm20_1 << 31) | (imm10_1_10 << 21) | (imm11_1 << 20) | (imm19_12 << 12);
  value |= rd << 7 | opcode;
  return value;
};

void j_type::set_value(uint32_t value) {
  imm21 = get_imm21(value);
  rd = bitcrop(value, 5, 7);
  opcode = bitcrop(value, 7, 0);
};

uint32_t get_opcode(uint32_t ir) { return bitcrop(ir, 7, 0); }

uint32_t get_rd(uint32_t ir) { return bitcrop(ir, 5, 7); }

uint32_t get_rs1(uint32_t ir) { return bitcrop(ir, 5, 15); }

uint32_t get_rs2(uint32_t ir) { return bitcrop(ir, 5, 20); }

uint32_t get_imm12(uint32_t ir) {
  uint32_t imm12 = bitcrop(ir, 12, 20);
  
  // Sign extend
  imm12 |= (imm12 & (1 << 11)) ? 0xFFFFF000 : 0;
  return imm12;
}

uint32_t get_imm13(uint32_t ir) {
  uint32_t imm13 = 0;
  imm13 |= bitshift(ir, 1, 31, 12) | bitshift(ir, 6, 25, 5);
  imm13 |= bitshift(ir, 4, 8, 1) | bitshift(ir, 1, 7, 11);

  // Sign extend
  imm13 |= (imm13 & (1 << 12)) ? 0xFFFFE000 : 0;
  return imm13;
}

uint32_t get_imm21(uint32_t ir) {
  uint32_t imm21 = 0;
  imm21 |= (bitcrop(ir, 1, 31) << 20) | (bitcrop(ir, 8, 12) << 12);
  imm21 |= (bitcrop(ir, 1, 20) << 11) | (bitcrop(ir, 10, 21) << 1);

  // Sign extend
  imm21 |= (imm21 & (1 << 20)) ? 0xFFE00000 : 0;
  return imm21;
}
