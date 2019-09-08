#include "load_assembler.h"
#include "RISCV_Emulator.h"
#include "RISCV_cpu.h"
#include "bit_tools.h"
#include "instruction_encdec.h"

using namespace std;

void load_assembler(uint32_t *mem) {
  mem[0] = asm_addi(T0, ZERO, 0);
  mem[1] = asm_addi(T1, ZERO, 0);
  mem[2] = asm_addi(T2, ZERO, 10);
  mem[3] = asm_addi(T0, T0, 1);
  mem[4] = asm_add(T1, T1, T0);
  mem[5] = asm_beq(T0, T2, 8);
  mem[6] = asm_jal(ZERO, -16);
  mem[7] = asm_add(A0, T1, ZERO);
  mem[8] = asm_jalr(ZERO, RA, 0);
}

// R_TYPE
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

uint32_t asm_sub(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  r_type cmd;
  cmd.funct7 = FUNC_SUB;
  cmd.opcode = OPCODE_ADD;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.rd = rd;
  cmd.funct3 = FUNC3_SUB;
  return cmd.value();
}

// I_TYPE
uint32_t asm_addi(uint32_t rd, uint32_t rs1, int32_t imm12) {
  i_type cmd;
  cmd.imm12 = imm12;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.opcode = OPCODE_ADDI;
  cmd.funct3 = FUNC3_ADDI;
  return cmd.value();
}

uint32_t asm_slli(uint32_t rd, uint32_t rs1, int32_t imm12) {
  i_type cmd;
  cmd.imm12 = imm12;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.opcode = OPCODE_ADDI;
  cmd.funct3 = FUNC3_SLLI;
  return cmd.value();
}

uint32_t asm_jalr(uint32_t rd, uint32_t rs1, int32_t offset12) {
  i_type cmd;
  cmd.opcode = OPCODE_JALR;
  cmd.funct3 = FUNC3_JALR;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.value();
}

uint32_t asm_lw(uint32_t rd, uint32_t rs1, int32_t offset12) {
  i_type cmd;
  cmd.opcode = OPCODE_LD;
  cmd.funct3 = FUNC3_LW;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.value();
}

// B TYPE
uint32_t asm_beq(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  b_type cmd;
  cmd.opcode = OPCODE_B;
  cmd.funct3 = FUNC3_BEQ;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.imm13 = offset13;
  return cmd.value();
}

uint32_t asm_bge(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  b_type cmd;
  cmd.opcode = OPCODE_B;
  cmd.funct3 = FUNC3_BGE;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.imm13 = offset13;
  return cmd.value();
}

uint32_t asm_bltu(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  b_type cmd;
  cmd.opcode = OPCODE_B;
  cmd.funct3 = FUNC3_BLTU;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.imm13 = offset13;
  return cmd.value();
}

uint32_t asm_bne(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  b_type cmd;
  cmd.opcode = OPCODE_B;
  cmd.funct3 = FUNC3_BNE;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.imm13 = offset13;
  return cmd.value();
}

// J TYPE
uint32_t asm_jal(uint32_t rd, int32_t offset21) {
  j_type cmd;
  cmd.opcode = OPCODE_J;
  cmd.rd = rd;
  cmd.imm21 = offset21;
  return cmd.value();
}

// S TYPE
uint32_t asm_sw(uint32_t rs1, uint32_t rs2, int32_t offset12) {
  s_type cmd;
  cmd.opcode = OPCODE_S;
  cmd.funct3 = FUNC3_SW;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.value();
}
