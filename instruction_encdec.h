#ifndef INSTRUCTION_ENCDEC_H
#define INSTRUCTION_ENCDEC_H

#include <cstdint>

namespace RISCV_EMULATOR {

class BitField {
public:
  virtual uint32_t GetValue() = 0;
  virtual void SetValue(uint32_t value) = 0;
  uint8_t opcode: 7;
  uint8_t rd: 5;
  uint8_t rs2: 5;
  uint8_t rs1: 5;
  uint8_t funct3: 3;
};

class RType : public BitField {
public:
  uint8_t funct7: 7;
  uint32_t GetValue() override;
  void SetValue(uint32_t value) override;
};

class IType : public BitField {
public:
  int16_t imm12: 12;
  uint32_t GetValue() override ;
  void SetValue(uint32_t value) override;
};

class SType : public BitField {
public:
  int16_t imm12: 12;
  uint32_t GetValue() override ;
  void SetValue(uint32_t value) override;
};

class BType : public BitField {
public:
  int16_t imm13: 13;
  uint32_t GetValue() override ;
  void SetValue(uint32_t value) override;
};

class JType : public BitField {
public:
  int32_t imm21: 21;
  uint32_t GetValue() override ;
  void SetValue(uint32_t value) override;
};

class UType : public BitField {
public:
  uint32_t imm20: 24;
  uint32_t GetValue() override ;
  void SetValue(uint32_t value) override;
};

uint32_t GetOpcode(uint32_t ir);

uint32_t GetRd(uint32_t ir);

uint32_t GetRs1(uint32_t ir);

uint32_t GetRs2(uint32_t ir);

int32_t GetImm12(uint32_t ir);

int32_t GetCsr(uint32_t ir);

int32_t GetImm13(uint32_t ir);

int32_t GetImm21(uint32_t ir);

uint32_t GetImm20(uint32_t ir);

int32_t GetStypeImm12(uint32_t ir);

uint32_t GetShamt(uint32_t ir);

} // namespace // RISCV_EMULATOR

#endif // INSTRUCTION_ENCDEC_H