#include "load_assembler.h"
#include "RISCV_cpu.h"
#include "instruction_encdec.h"
#include <cstdint>
#include <cassert>

// R_TYPE
uint32_t AsmAdd(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  RType cmd;
  cmd.funct7 = FUNC_NORM;
  cmd.opcode = OPCODE_ARITHLOG;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.rd = rd;
  cmd.funct3 = FUNC3_ADDSUB;
  return cmd.GetValue();
}

uint32_t AsmSub(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  RType cmd;
  cmd.funct7 = FUNC_ALT;
  cmd.opcode = OPCODE_ARITHLOG;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.rd = rd;
  cmd.funct3 = FUNC3_ADDSUB;
  return cmd.GetValue();
}

uint32_t AsmAnd(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  RType cmd;
  cmd.funct7 = FUNC_NORM;
  cmd.opcode = OPCODE_ARITHLOG;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.rd = rd;
  cmd.funct3 = FUNC3_AND;
  return cmd.GetValue();
}

uint32_t AsmOr(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  RType cmd;
  cmd.funct7 = FUNC_NORM;
  cmd.opcode = OPCODE_ARITHLOG;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.rd = rd;
  cmd.funct3 = FUNC3_OR;
  return cmd.GetValue();
}

uint32_t AsmXor(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  RType cmd;
  cmd.funct7 = FUNC_NORM;
  cmd.opcode = OPCODE_ARITHLOG;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.rd = rd;
  cmd.funct3 = FUNC3_XOR;
  return cmd.GetValue();
}

uint32_t AsmSll(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  RType cmd;
  cmd.funct7 = FUNC_NORM;
  cmd.opcode = OPCODE_ARITHLOG;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.rd = rd;
  cmd.funct3 = FUNC3_SL;
  return cmd.GetValue();
}

uint32_t AsmSrl(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  RType cmd;
  cmd.funct7 = FUNC_NORM;
  cmd.opcode = OPCODE_ARITHLOG;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.rd = rd;
  cmd.funct3 = FUNC3_SR;
  return cmd.GetValue();
}

uint32_t AsmSra(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  RType cmd;
  cmd.funct7 = FUNC_ALT;
  cmd.opcode = OPCODE_ARITHLOG;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.rd = rd;
  cmd.funct3 = FUNC3_SR;
  return cmd.GetValue();
}

uint32_t AsmSlt(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  RType cmd;
  cmd.funct7 = FUNC_NORM;
  cmd.opcode = OPCODE_ARITHLOG;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.rd = rd;
  cmd.funct3 = FUNC3_SLT;
  return cmd.GetValue();
}

uint32_t AsmSltu(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  RType cmd;
  cmd.funct7 = FUNC_NORM;
  cmd.opcode = OPCODE_ARITHLOG;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.rd = rd;
  cmd.funct3 = FUNC3_SLTU;
  return cmd.GetValue();
}

uint32_t AsmMret() {
  RType cmd;
  cmd.funct7 = FUNC_MRET;
  cmd.opcode = OPCODE_SYSTEM;
  cmd.rs2 = 0b00010; // fixed for MRET
  cmd.rs1 = 0;
  cmd.rd = 0;
  cmd.funct3 = FUNC3_SYSTEM;
  return cmd.GetValue();
}

// I_TYPE
uint32_t AsmAddi(uint32_t rd, uint32_t rs1, int32_t imm12) {
  IType cmd;
  cmd.imm12 = imm12 & 0xFFF;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.opcode = OPCODE_ARITHLOG_I;
  cmd.funct3 = FUNC3_ADDSUB;
  return cmd.GetValue();
}

uint32_t AsmAndi(uint32_t rd, uint32_t rs1, int32_t imm12) {
  IType cmd;
  cmd.imm12 = imm12 & 0xFFF;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.opcode = OPCODE_ARITHLOG_I;
  cmd.funct3 = FUNC3_AND;
  return cmd.GetValue();
}

uint32_t AsmOri(uint32_t rd, uint32_t rs1, int32_t imm12) {
  IType cmd;
  cmd.imm12 = imm12 & 0xFFF;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.opcode = OPCODE_ARITHLOG_I;
  cmd.funct3 = FUNC3_OR;
  return cmd.GetValue();
}

uint32_t AsmXori(uint32_t rd, uint32_t rs1, int32_t imm12) {
  IType cmd;
  cmd.imm12 = imm12 & 0xFFF;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.opcode = OPCODE_ARITHLOG_I;
  cmd.funct3 = FUNC3_XOR;
  return cmd.GetValue();
}

uint32_t AsmSlli(uint32_t rd, uint32_t rs1, int32_t imm12) {
  IType cmd;
  // SLLI immediate is 6 bit wide.
  imm12 &= 0b0111111;
  cmd.imm12 = imm12 & 0b0111111;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.opcode = OPCODE_ARITHLOG_I;
  cmd.funct3 = FUNC3_SL;
  return cmd.GetValue();
}

uint32_t AsmSrli(uint32_t rd, uint32_t rs1, int32_t imm12) {
  IType cmd;
  // SRLI immediate is 6 bit wide.
  cmd.imm12 = imm12 & 0b0111111;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.opcode = OPCODE_ARITHLOG_I;
  cmd.funct3 = FUNC3_SR;
  return cmd.GetValue();
}

uint32_t AsmSrai(uint32_t rd, uint32_t rs1, int32_t imm12) {
  IType cmd;
  // SRAI immediate is 6 bit wide.
  imm12 &= 0b0111111;
  // SRAI imm12 top 6 bit is same as funct7
  imm12 |= (FUNC_ALT >> 1) << 6;
  cmd.imm12 = imm12;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.opcode = OPCODE_ARITHLOG_I;
  cmd.funct3 = FUNC3_SR;
  return cmd.GetValue();
}

uint32_t AsmSlti(uint32_t rd, uint32_t rs1, int32_t imm12) {
  IType cmd;
  cmd.imm12 = imm12;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.opcode = OPCODE_ARITHLOG_I;
  cmd.funct3 = FUNC3_SLT;
  return cmd.GetValue();
}

uint32_t AsmSltiu(uint32_t rd, uint32_t rs1, int32_t imm12) {
  IType cmd;
  cmd.imm12 = imm12;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.opcode = OPCODE_ARITHLOG_I;
  cmd.funct3 = FUNC3_SLTU;
  return cmd.GetValue();
}

uint32_t AsmJalr(uint32_t rd, uint32_t rs1, int32_t offset12) {
  IType cmd;
  cmd.opcode = OPCODE_JALR;
  cmd.funct3 = FUNC3_JALR;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}

uint32_t AsmEbreak() {
  IType cmd;
  cmd.opcode = OPCODE_SYSTEM;
  cmd.funct3 = FUNC3_SYSTEM;
  cmd.rd = 0;
  cmd.rs1 = 0;
  cmd.imm12 = 1;
  return cmd.GetValue();
}

uint32_t AsmEcall() {
  IType cmd;
  cmd.opcode = OPCODE_SYSTEM;
  cmd.funct3 = FUNC3_SYSTEM;
  cmd.rd = 0;
  cmd.rs1 = 0;
  cmd.imm12 = 0;
  return cmd.GetValue();
}

uint32_t AsmCsrrc(uint32_t rd, uint32_t rs1, int32_t offset12) {
  IType cmd;
  cmd.opcode = OPCODE_SYSTEM;
  cmd.funct3 = FUNC3_CSRRC;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}

uint32_t AsmCsrrci(uint32_t rd, uint32_t zimm, int32_t offset12) {
  IType cmd;
  cmd.opcode = OPCODE_SYSTEM;
  cmd.funct3 = FUNC3_CSRRCI;
  cmd.rd = rd;
  cmd.rs1 = zimm;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}
uint32_t AsmCsrrs(uint32_t rd, uint32_t rs1, int32_t offset12) {
  IType cmd;
  cmd.opcode = OPCODE_SYSTEM;
  cmd.funct3 = FUNC3_CSRRS;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}

uint32_t AsmCsrrsi(uint32_t rd, uint32_t zimm, int32_t offset12) {
  IType cmd;
  cmd.opcode = OPCODE_SYSTEM;
  cmd.funct3 = FUNC3_CSRRSI;
  cmd.rd = rd;
  cmd.rs1 = zimm;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}


uint32_t AsmCsrrw(uint32_t rd, uint32_t rs1, int32_t offset12) {
  IType cmd;
  cmd.opcode = OPCODE_SYSTEM;
  cmd.funct3 = FUNC3_CSRRW;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}

uint32_t AsmCsrrwi(uint32_t rd, uint32_t zimm, int32_t offset12) {
  IType cmd;
  cmd.opcode = OPCODE_SYSTEM;
  cmd.funct3 = FUNC3_CSRRWI;
  cmd.rd = rd;
  cmd.rs1 = zimm;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}

uint32_t AsmFence(uint32_t pred, uint32_t succ) {
  IType cmd;
  assert((pred & ~0xF) == 0);
  assert((succ & ~0xF) == 0);
  uint32_t imm12 = (pred << 4) | succ;
  cmd.opcode = OPCODE_FENCE;
  cmd.funct3 = FUNC3_FENCE;
  cmd.imm12 = imm12;
  cmd.rd = 0;
  cmd.rs1 = 0;
  return cmd.GetValue();
}

uint32_t AsmFencei() {
  IType cmd;
  cmd.opcode = OPCODE_FENCE;
  cmd.funct3 = FUNC3_FENCEI;
  cmd.imm12 = 0;
  cmd.rd = 0;
  cmd.rs1 = 0;
  return cmd.GetValue();
}

uint32_t AsmLb(uint32_t rd, uint32_t rs1, int32_t offset12) {
  IType cmd;
  cmd.opcode = OPCODE_LD;
  cmd.funct3 = FUNC3_LSB;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}

uint32_t AsmLbu(uint32_t rd, uint32_t rs1, int32_t offset12) {
  IType cmd;
  cmd.opcode = OPCODE_LD;
  cmd.funct3 = FUNC3_LSBU;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}

uint32_t AsmLh(uint32_t rd, uint32_t rs1, int32_t offset12) {
  IType cmd;
  cmd.opcode = OPCODE_LD;
  cmd.funct3 = FUNC3_LSH;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}

uint32_t AsmLhu(uint32_t rd, uint32_t rs1, int32_t offset12) {
  IType cmd;
  cmd.opcode = OPCODE_LD;
  cmd.funct3 = FUNC3_LSHU;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}

uint32_t AsmLw(uint32_t rd, uint32_t rs1, int32_t offset12) {
  IType cmd;
  cmd.opcode = OPCODE_LD;
  cmd.funct3 = FUNC3_LSW;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}

// B TYPE
uint32_t AsmBeq(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  BType cmd;
  cmd.opcode = OPCODE_B;
  cmd.funct3 = FUNC3_BEQ;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.imm13 = offset13;
  return cmd.GetValue();
}

uint32_t AsmBge(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  BType cmd;
  cmd.opcode = OPCODE_B;
  cmd.funct3 = FUNC3_BGE;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.imm13 = offset13;
  return cmd.GetValue();
}

uint32_t AsmBgeu(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  BType cmd;
  cmd.opcode = OPCODE_B;
  cmd.funct3 = FUNC3_BGEU;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.imm13 = offset13;
  return cmd.GetValue();
}

uint32_t AsmBlt(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  BType cmd;
  cmd.opcode = OPCODE_B;
  cmd.funct3 = FUNC3_BLT;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.imm13 = offset13;
  return cmd.GetValue();
}

uint32_t AsmBltu(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  BType cmd;
  cmd.opcode = OPCODE_B;
  cmd.funct3 = FUNC3_BLTU;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.imm13 = offset13;
  return cmd.GetValue();
}

uint32_t AsmBne(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  BType cmd;
  cmd.opcode = OPCODE_B;
  cmd.funct3 = FUNC3_BNE;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.imm13 = offset13;
  return cmd.GetValue();
}

// J TYPE
uint32_t AsmJal(uint32_t rd, int32_t offset21) {
  JType cmd;
  cmd.opcode = OPCODE_J;
  cmd.rd = rd;
  cmd.imm21 = offset21;
  return cmd.GetValue();
}

// S TYPE
uint32_t AsmSw(uint32_t rs1, uint32_t rs2, int32_t offset12) {
  SType cmd;
  cmd.opcode = OPCODE_S;
  cmd.funct3 = FUNC3_LSW;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}

uint32_t AsmSh(uint32_t rs1, uint32_t rs2, int32_t offset12) {
  SType cmd;
  cmd.opcode = OPCODE_S;
  cmd.funct3 = FUNC3_LSH;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}

uint32_t AsmSb(uint32_t rs1, uint32_t rs2, int32_t offset12) {
  SType cmd;
  cmd.opcode = OPCODE_S;
  cmd.funct3 = FUNC3_LSB;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}

// U TYPE
uint32_t AsmLui(uint32_t rd, int32_t imm20) {
  UType cmd;
  cmd.opcode = OPCODE_LUI;
  cmd.rd = rd;
  cmd.imm20 = imm20 & 0x0FFFFF;
  return cmd.GetValue();
}

uint32_t AsmAuipc(uint32_t rd, int32_t imm20) {
  UType cmd;
  cmd.opcode = OPCODE_AUIPC;
  cmd.rd = rd;
  cmd.imm20 = imm20 & 0x0FFFFF;
  return cmd.GetValue();
}
