#ifndef INSTRUCTION_ENCDEC_H
#define INSTRUCTION_ENCDEC_H

#include <cstdint>

class bitfield {
  public:
    virtual uint32_t value() = 0;
    virtual void set_value(uint32_t value) = 0;

    uint8_t opcode: 7;
    uint8_t rd: 5;
    uint8_t rs2: 5;
    uint8_t rs1: 5;
    uint8_t funct3: 3;
};

class r_type : public bitfield {
  public:
    uint8_t funct7: 7;
    uint32_t value();
    void set_value(uint32_t value) override;
};

class i_type : public bitfield {
  public:
    int16_t imm12 : 12;
    uint32_t value();
    void set_value(uint32_t value) override;
};

class s_type : public bitfield{
  public:
    int16_t imm12 : 12;
    uint32_t value();
    void set_value(uint32_t value) override;
};

class b_type : public bitfield {
  public:
    int16_t imm13 : 13;
    uint32_t value();
    void set_value(uint32_t value) override;
};

class u_type: public bitfield {
  public:
    int32_t imm32 : 32;
    uint32_t value();
    void set_value(uint32_t value) override;
};

class j_type : public bitfield {
  public:
    int32_t imm21 : 21;
    uint32_t value();
    void set_value(uint32_t value) override;
};

uint32_t get_opcode(uint32_t ir);
uint32_t get_rd(uint32_t ir);
uint32_t get_rs1(uint32_t ir);
uint32_t get_rs2(uint32_t ir);
uint32_t get_imm12(uint32_t ir);
uint32_t get_imm13(uint32_t ir);
uint32_t get_imm21(uint32_t ir);
uint32_t get_stype_imm12(uint32_t ir);
uint32_t get_shamt(uint32_t ir);

#endif // INSTRUCTION_ENCDEC_H