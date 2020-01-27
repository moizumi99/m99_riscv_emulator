#include "instruction_encdec.h"
#include "bit_tools.h"
#include "RISCV_cpu.h"
#include <cstdint>

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
  opcode = bitcrop(value, 7, 0);
  funct3 = bitcrop(value, 3, 12);
  imm12 = get_imm12(value);
  // For SLLI, SRLI, SRAI, only the lower 6 bits are relevant.
  if (opcode == OPCODE_ARITHLOG_I && (funct3 == FUNC3_SR || funct3 == FUNC3_SL)) {
    imm12 &= 0b0111111;
  }
  rs1 = bitcrop(value, 5, 15);
  rd = bitcrop(value, 5, 7);
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
  imm12 = get_stype_imm12(value);
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
  value |= (imm20 & (0xFFFFF)) << 12;
  value |= rd << 7 | opcode;
  return value;
};

void u_type::set_value(uint32_t value) {
  imm20 = get_imm20(value);
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

int32_t get_imm12(uint32_t ir) {
  int32_t imm12 = sext(bitcrop(ir, 12, 20), 12);
  return imm12;
}

int32_t get_csr(uint32_t ir) {
  int32_t csr = bitcrop(ir, 12, 20);
  return csr;
}

int32_t get_imm13(uint32_t ir) {
  int32_t imm13 = 0;
  imm13 |= bitshift(ir, 1, 31, 12) | bitshift(ir, 6, 25, 5);
  imm13 |= bitshift(ir, 4, 8, 1) | bitshift(ir, 1, 7, 11);
  imm13 = sext(imm13, 13);
  return imm13;
}

int32_t get_imm21(uint32_t ir) {
  int32_t imm21 = 0;
  imm21 |= (bitcrop(ir, 1, 31) << 20) | (bitcrop(ir, 8, 12) << 12);
  imm21 |= (bitcrop(ir, 1, 20) << 11) | (bitcrop(ir, 10, 21) << 1);

  imm21 = sext(imm21, 21);
  return imm21;
}

int32_t get_stype_imm12(uint32_t ir) {
  int32_t imm12 = (bitcrop(ir, 7, 25) << 5) | bitcrop(ir, 5, 7);
  imm12 = sext(imm12, 12);
  return imm12;
}

uint32_t get_shamt(uint32_t ir) {
  uint32_t shamt = bitcrop(ir, 5, 20);
  return shamt;
}

uint32_t get_imm20(uint32_t ir) {
  uint32_t imm20 = bitcrop(ir, 20, 12);
  imm20 = sext(imm20, 20);
  return imm20;
}
