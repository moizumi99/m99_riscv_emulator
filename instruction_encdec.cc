#include "instruction_encdec.h"
#include "bit_tools.h"
#include "RISCV_cpu.h"
#include <cstdint>

namespace RISCV_EMULATOR {

uint32_t RType::GetValue() {
  uint32_t value = 0;
  value |= funct7 << 25 | rs2 << 20 | rs1 << 15;
  value |= funct3 << 12 | rd << 7 | opcode;
  return value;
};

void RType::SetValue(uint32_t value) {
  funct7 = bitcrop(value, 7, 25);
  rs2 = bitcrop(value, 5, 20);
  rs1 = bitcrop(value, 5, 15);
  funct3 = bitcrop(value, 3, 12);
  rd = bitcrop(value, 5, 7);
  opcode = bitcrop(value, 7, 0);
};

uint32_t IType::GetValue() {
  uint32_t value = 0;
  value |= imm12 << 20 | rs1 << 15;
  value |= funct3 << 12 | rd << 7 | opcode;
  return value;
};

void IType::SetValue(uint32_t value) {
  opcode = bitcrop(value, 7, 0);
  funct3 = bitcrop(value, 3, 12);
  imm12 = GetImm12(value);
  // For SLLIW, SRLIW, SRAIW, only the lower 6 bits are relevant.
  if ((opcode == OPCODE_ARITHLOG_I || opcode == OPCODE_ARITHLOG_I64) &&
      (funct3 == FUNC3_SR || funct3 == FUNC3_SL)) {
    imm12 &= 0b0111111;
  }
  rs1 = bitcrop(value, 5, 15);
  rd = bitcrop(value, 5, 7);
};

uint32_t SType::GetValue() {
  uint32_t value = 0;
  uint32_t imm11_5_7 = bitcrop(imm12, 7, 5);
  uint32_t imm4_0_5 = bitcrop(imm12, 5, 0);
  value |= imm11_5_7 << 25 | rs2 << 20 | rs1 << 15;
  value |= funct3 << 12 | imm4_0_5 << 7 | opcode;
  return value;
};

void SType::SetValue(uint32_t value) {
  rs2 = bitcrop(value, 5, 20);
  rs1 = bitcrop(value, 5, 15);
  funct3 = bitcrop(value, 3, 12);
  opcode = bitcrop(value, 7, 0);
  imm12 = GetStypeImm12(value);
};

uint32_t BType::GetValue() {
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

void BType::SetValue(uint32_t value) {
  rs2 = bitcrop(value, 5, 20);
  rs1 = bitcrop(value, 5, 15);
  funct3 = bitcrop(value, 3, 12);
  opcode = bitcrop(value, 7, 0);
  imm13 = GetImm13(value);
};

uint32_t UType::GetValue() {
  uint32_t value = 0;
  value |= (imm20 & (0xFFFFF)) << 12;
  value |= rd << 7 | opcode;
  return value;
};

void UType::SetValue(uint32_t value) {
  imm20 = GetImm20(value);
  rd = bitcrop(value, 5, 7);
  opcode = bitcrop(value, 7, 0);
};

uint32_t JType::GetValue() {
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

void JType::SetValue(uint32_t value) {
  imm21 = GetImm21(value);
  rd = bitcrop(value, 5, 7);
  opcode = bitcrop(value, 7, 0);
};

uint32_t GetOpcode(uint32_t ir) { return bitcrop(ir, 7, 0); }

uint32_t GetRd(uint32_t ir) { return bitcrop(ir, 5, 7); }

uint32_t GetRs1(uint32_t ir) { return bitcrop(ir, 5, 15); }

uint32_t GetRs2(uint32_t ir) { return bitcrop(ir, 5, 20); }

int32_t GetImm12(uint32_t ir) {
  int32_t imm12 = SignExtend(bitcrop(ir, 12, 20), 12);
  return imm12;
}

int32_t GetCsr(uint32_t ir) {
  int32_t csr = bitcrop(ir, 12, 20);
  return csr;
}

int32_t GetImm(uint32_t ir) {
  int32_t imm = 0;
  uint16_t opcode = bitcrop(ir, 7, 0);
  uint8_t funct3 = bitcrop(ir, 3, 12);
  imm = GetImm12(ir);
  switch (opcode) {
    case OPCODE_ARITHLOG_I:
    case OPCODE_ARITHLOG_I64:
      if (funct3 == FUNC3_SL || funct3 == FUNC3_SR) {
        imm = imm & 0b111111;
      }
      break;
    case OPCODE_B:
      imm = GetImm13(ir);
      break;
    case OPCODE_LUI:
    case OPCODE_AUIPC:
      imm = GetImm20(ir) << 12;
      break;
    case OPCODE_J:
      imm = GetImm21(ir);
      break;
    case OPCODE_S:
      imm = GetStypeImm12(ir);
      break;
  }
  return imm;
}

int32_t GetImm13(uint32_t ir) {
  int32_t imm13 = 0;
  imm13 |= BitShift(ir, 1, 31, 12) | BitShift(ir, 6, 25, 5);
  imm13 |= BitShift(ir, 4, 8, 1) | BitShift(ir, 1, 7, 11);
  imm13 = SignExtend(imm13, 13);
  return imm13;
}

int32_t GetImm21(uint32_t ir) {
  int32_t imm21 = 0;
  uint32_t offset19_12 = bitcrop(ir, 8, 12);
  uint32_t offset11 = bitcrop(ir, 1, 20);
  uint32_t offset10_1 = bitcrop(ir, 10, 21);
  uint32_t offset20 = bitcrop(ir, 1, 31);
  imm21 = (offset20 << 20) | (offset19_12 << 12) | (offset11 << 11) |
          (offset10_1 << 1);
//  imm21 = (bitcrop(ir, 1, 31) << 20) | (bitcrop(ir, 8, 12) << 12);
//  imm21 |= (bitcrop(ir, 1, 20) << 11) | (bitcrop(ir, 10, 21) << 1);

  imm21 = SignExtend(imm21, 21);
  return imm21;
}

int32_t GetStypeImm12(uint32_t ir) {
  int32_t imm12 = (bitcrop(ir, 7, 25) << 5) | bitcrop(ir, 5, 7);
  imm12 = SignExtend(imm12, 12);
  return imm12;
}

uint32_t GetShamt(uint32_t ir) {
  uint32_t shamt = bitcrop(ir, 6, 20);
  return shamt;
}

uint32_t GetImm20(uint32_t ir) {
  uint32_t imm20 = bitcrop(ir, 20, 12);
  imm20 = SignExtend(imm20, 20);
  return imm20;
}

} // namespace RISCV_EMULATOR