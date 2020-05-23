#include "load_assembler.h"
#include "RISCV_cpu.h"
#include "instruction_encdec.h"
#include <cstdint>
#include <cassert>

namespace CPU_TEST {

// R_TYPE
uint32_t AsmRType(op_label opcode, op_funct funct, op_funct3 funct3, uint32_t rd, uint32_t rs1, uint32_t rs2) {
  RType cmd;
  cmd.opcode = opcode;
  cmd.funct7 = funct;
  cmd.funct3 = funct3;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.rs2 = rs2;
  return  cmd.GetValue();
}

uint32_t AsmAdd(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_NORM, FUNC3_ADDSUB, rd, rs1, rs2);
}

uint32_t AsmAddw(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG_64, FUNC_NORM, FUNC3_ADDSUB, rd, rs1, rs2);
}

uint32_t AsmSub(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_ALT, FUNC3_ADDSUB, rd, rs1, rs2);
}

uint32_t AsmSubw(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG_64, FUNC_ALT, FUNC3_ADDSUB, rd, rs1, rs2);
}

uint32_t AsmAnd(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_NORM, FUNC3_AND, rd, rs1, rs2);
}

uint32_t AsmOr(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_NORM, FUNC3_OR, rd, rs1, rs2);
}

uint32_t AsmXor(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_NORM, FUNC3_XOR, rd, rs1, rs2);
}

uint32_t AsmSll(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_NORM, FUNC3_SL, rd, rs1, rs2);
}

uint32_t AsmSllw(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG_64, FUNC_NORM, FUNC3_SL, rd, rs1, rs2);
}

uint32_t AsmSrl(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_NORM, FUNC3_SR, rd, rs1, rs2);
}

uint32_t AsmSrlw(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG_64, FUNC_NORM, FUNC3_SR, rd, rs1, rs2);
}

uint32_t AsmSra(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_ALT, FUNC3_SR, rd, rs1, rs2);
}

uint32_t AsmSraw(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG_64, FUNC_ALT, FUNC3_SR, rd, rs1, rs2);
}

uint32_t AsmSlt(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_NORM, FUNC3_SLT, rd, rs1, rs2);
}

uint32_t AsmSltu(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_NORM, FUNC3_SLTU, rd, rs1, rs2);
}

uint32_t AsmMret() {
  uint32_t rs2 = 0b00010; // fixed for MRET
  return AsmRType(OPCODE_SYSTEM, FUNC_MRET, FUNC3_SYSTEM, 0, 0, rs2);
}

// I_TYPE
uint32_t AsmIType(op_label opcode, op_funct3 funct3, uint32_t rd, uint32_t rs1, int32_t imm12) {
  IType cmd;
  cmd.opcode = opcode;
  cmd.funct3 = funct3;
  cmd.imm12 = imm12 & 0xFFF;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  return cmd.GetValue();
}

uint32_t AsmAddi(uint32_t rd, uint32_t rs1, int32_t imm12) {
  return AsmIType(OPCODE_ARITHLOG_I, FUNC3_ADDSUB, rd, rs1, imm12);
}

uint32_t AsmAddiw(uint32_t rd, uint32_t rs1, int32_t imm12) {
  return AsmIType(OPCODE_ARITHLOG_I64, FUNC3_ADDSUB, rd, rs1, imm12);
}

uint32_t AsmAndi(uint32_t rd, uint32_t rs1, int32_t imm12) {
  return AsmIType(OPCODE_ARITHLOG_I, FUNC3_AND, rd, rs1, imm12);
}

uint32_t AsmOri(uint32_t rd, uint32_t rs1, int32_t imm12) {
  return AsmIType(OPCODE_ARITHLOG_I, FUNC3_OR, rd, rs1, imm12);
}

uint32_t AsmXori(uint32_t rd, uint32_t rs1, int32_t imm12) {
  return AsmIType(OPCODE_ARITHLOG_I, FUNC3_XOR, rd, rs1, imm12);
}

uint32_t AsmSlli(uint32_t rd, uint32_t rs1, int32_t imm12) {
  // SLLI immediate is 6 bit wide.
  imm12 &= 0b0111111;
  return AsmIType(OPCODE_ARITHLOG_I, FUNC3_SL, rd, rs1, imm12);
}

uint32_t AsmSlliw(uint32_t rd, uint32_t rs1, int32_t imm12) {
  // SLLI immediate is 6 bit wide.
  imm12 &= 0b0111111;
  return AsmIType(OPCODE_ARITHLOG_I64, FUNC3_SL, rd, rs1, imm12);
}

uint32_t AsmSrli(uint32_t rd, uint32_t rs1, int32_t imm12) {
  // SRLI immediate is 6 bit wide.
  imm12 = imm12 & 0b0111111;
  return AsmIType(OPCODE_ARITHLOG_I, FUNC3_SR, rd, rs1, imm12);
}

uint32_t AsmSrliw(uint32_t rd, uint32_t rs1, int32_t imm12) {
  // SRLI immediate is 6 bit wide.
  imm12 = imm12 & 0b0111111;
  return AsmIType(OPCODE_ARITHLOG_I64, FUNC3_SR, rd, rs1, imm12);
}

uint32_t AsmSrai(uint32_t rd, uint32_t rs1, int32_t imm12) {
  IType cmd;
  // SRAI immediate is 6 bit wide.
  imm12 &= 0b0111111;
  // SRAI imm12 top 6 bit is same as funct7
  imm12 |= (FUNC_ALT >> 1) << 6;
  return AsmIType(OPCODE_ARITHLOG_I, FUNC3_SR, rd, rs1, imm12);
}

uint32_t AsmSraiw(uint32_t rd, uint32_t rs1, int32_t imm12) {
  IType cmd;
  // SRAIW immediate is 6 bit wide.
  imm12 &= 0b0111111;
  // SRAIW imm12 top 6 bit is same as funct7
  imm12 |= (FUNC_ALT >> 1) << 6;
  return AsmIType(OPCODE_ARITHLOG_I64, FUNC3_SR, rd, rs1, imm12);
}

uint32_t AsmSlti(uint32_t rd, uint32_t rs1, int32_t imm12) {
  return AsmIType(OPCODE_ARITHLOG_I, FUNC3_SLT, rd, rs1, imm12);
}

uint32_t AsmSltiu(uint32_t rd, uint32_t rs1, int32_t imm12) {
  return AsmIType(OPCODE_ARITHLOG_I, FUNC3_SLTU, rd, rs1, imm12);
}

uint32_t AsmJalr(uint32_t rd, uint32_t rs1, int32_t offset12) {
  return AsmIType(OPCODE_JALR, FUNC3_JALR, rd, rs1, offset12);
}

uint32_t AsmEbreak() {
  return AsmIType(OPCODE_SYSTEM, FUNC3_SYSTEM, 0, 0, 1);
}

uint32_t AsmEcall() {
  return AsmIType(OPCODE_SYSTEM, FUNC3_SYSTEM, 0, 0, 0);
}

uint32_t AsmCsrrc(uint32_t rd, uint32_t rs1, int32_t offset12) {
  return AsmIType(OPCODE_SYSTEM, FUNC3_CSRRC, rd, rs1, offset12);
}

uint32_t AsmCsrrci(uint32_t rd, uint32_t zimm, int32_t offset12) {
  return AsmIType(OPCODE_SYSTEM, FUNC3_CSRRCI, rd, zimm, offset12);
}

uint32_t AsmCsrrs(uint32_t rd, uint32_t rs1, int32_t offset12) {
  return AsmIType(OPCODE_SYSTEM, FUNC3_CSRRS, rd, rs1, offset12);
}

uint32_t AsmCsrrsi(uint32_t rd, uint32_t zimm, int32_t offset12) {
  return AsmIType(OPCODE_SYSTEM, FUNC3_CSRRSI, rd, zimm, offset12);
}

uint32_t AsmCsrrw(uint32_t rd, uint32_t rs1, int32_t offset12) {
  return AsmIType(OPCODE_SYSTEM, FUNC3_CSRRW, rd, rs1, offset12);
}

uint32_t AsmCsrrwi(uint32_t rd, uint32_t zimm, int32_t offset12) {
  return AsmIType(OPCODE_SYSTEM, FUNC3_CSRRWI, rd, zimm, offset12);
}

uint32_t AsmFence(uint32_t pred, uint32_t succ) {
  assert((pred & ~0xF) == 0);
  assert((succ & ~0xF) == 0);
  uint32_t imm12 = (pred << 4) | succ;
  return AsmIType(OPCODE_FENCE, FUNC3_FENCE, 0, 0, imm12);
}

uint32_t AsmFencei() {
  return AsmIType(OPCODE_FENCE, FUNC3_FENCEI, 0, 0, 0);
}

uint32_t AsmLb(uint32_t rd, uint32_t rs1, int32_t offset12) {
  return AsmIType(OPCODE_LD, FUNC3_LSB, rd, rs1, offset12);
}

uint32_t AsmLbu(uint32_t rd, uint32_t rs1, int32_t offset12) {
  return AsmIType(OPCODE_LD, FUNC3_LSBU, rd, rs1, offset12);
}

uint32_t AsmLh(uint32_t rd, uint32_t rs1, int32_t offset12) {
  return AsmIType(OPCODE_LD, FUNC3_LSH, rd, rs1, offset12);
}

uint32_t AsmLhu(uint32_t rd, uint32_t rs1, int32_t offset12) {
  return AsmIType(OPCODE_LD, FUNC3_LSHU, rd, rs1, offset12);
}

uint32_t AsmLw(uint32_t rd, uint32_t rs1, int32_t offset12) {
  return AsmIType(OPCODE_LD, FUNC3_LSW, rd, rs1, offset12);
}

uint32_t AsmLd(uint32_t rd, uint32_t rs1, int32_t offset12) {
  return AsmIType(OPCODE_LD, FUNC3_LSD, rd, rs1, offset12);
}

uint32_t AsmLwu(uint32_t rd, uint32_t rs1, int32_t offset12) {
  return AsmIType(OPCODE_LD, FUNC3_LSWU, rd, rs1, offset12);
}

// B TYPE
uint32_t AsmBType(op_label opcode, op_funct3 funct3, uint32_t rs1, uint32_t rs2, int32_t offset13) {
  BType cmd;
  cmd.opcode = opcode;
  cmd.funct3 = funct3;
  cmd.rs1 = rs1;
  cmd.rs2 = rs2;
  cmd.imm13 = offset13;
  return cmd.GetValue();
}

uint32_t AsmBeq(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  return AsmBType(OPCODE_B, FUNC3_BEQ, rs1, rs2, offset13);
}

uint32_t AsmBge(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  return AsmBType(OPCODE_B, FUNC3_BGE, rs1, rs2, offset13);
}

uint32_t AsmBgeu(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  return AsmBType(OPCODE_B, FUNC3_BGEU, rs1, rs2, offset13);
}

uint32_t AsmBlt(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  return AsmBType(OPCODE_B, FUNC3_BLT, rs1, rs2, offset13);
}

uint32_t AsmBltu(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  return AsmBType(OPCODE_B, FUNC3_BLTU, rs1, rs2, offset13);
}

uint32_t AsmBne(uint32_t rs1, uint32_t rs2, int32_t offset13) {
  return AsmBType(OPCODE_B, FUNC3_BNE, rs1, rs2, offset13);
}

// J TYPE
uint32_t AsmJType(op_label opcode, uint32_t rd, int32_t offset21) {
  JType cmd;
  cmd.opcode = opcode;
  cmd.rd = rd;
  cmd.imm21 = offset21;
  return cmd.GetValue();
}

uint32_t AsmJal(uint32_t rd, int32_t offset21) {
  return AsmJType(OPCODE_J, rd, offset21);
}

// S TYPE
uint32_t AsmSType(op_label opcode, op_funct3 funct3, uint32_t rs1, uint32_t rs2, int32_t offset12) {
  SType cmd;
  cmd.opcode = opcode;
  cmd.funct3 = funct3;
  cmd.rs2 = rs2;
  cmd.rs1 = rs1;
  cmd.imm12 = offset12;
  return cmd.GetValue();
}

uint32_t AsmSw(uint32_t rs1, uint32_t rs2, int32_t offset12) {
  return AsmSType(OPCODE_S, FUNC3_LSW, rs1, rs2, offset12);
}

uint32_t AsmSh(uint32_t rs1, uint32_t rs2, int32_t offset12) {
  return AsmSType(OPCODE_S, FUNC3_LSH, rs1, rs2, offset12);
}

uint32_t AsmSb(uint32_t rs1, uint32_t rs2, int32_t offset12) {
  return AsmSType(OPCODE_S, FUNC3_LSB, rs1, rs2, offset12);
}

uint32_t AsmSd(uint32_t rs1, uint32_t rs2, int32_t offset12) {
  return AsmSType(OPCODE_S, FUNC3_LSD, rs1, rs2, offset12);
}

// U TYPE.
uint32_t AsmUType(op_label opcode, uint32_t rd, int32_t imm20) {
  UType cmd;
  cmd.opcode = opcode;
  cmd.rd = rd;
  cmd.imm20 = imm20;
  return cmd.GetValue();
}

uint32_t AsmLui(uint32_t rd, int32_t imm20) {
  return AsmUType(OPCODE_LUI, rd, imm20);
}

uint32_t AsmAuipc(uint32_t rd, int32_t imm20) {
  return AsmUType(OPCODE_AUIPC, rd, imm20);
}

// MULT Instructions (R-Type).
uint32_t AsmMul(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_MULT, FUNC3_MUL, rd, rs1, rs2);
}

uint32_t AsmMulh(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_MULT, FUNC3_MULH, rd, rs1, rs2);
}

uint32_t AsmMulhsu(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_MULT, FUNC3_MULHSU, rd, rs1, rs2);
}

uint32_t AsmMulhu(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_MULT, FUNC3_MULHU, rd, rs1, rs2);
}

uint32_t AsmMulw(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG_64, FUNC_MULT, FUNC3_MUL, rd, rs1, rs2);
}

uint32_t AsmDiv(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_MULT, FUNC3_DIV, rd, rs1, rs2);
}

uint32_t AsmDivu(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_MULT, FUNC3_DIVU, rd, rs1, rs2);
}

uint32_t AsmDivuw(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG_64, FUNC_MULT, FUNC3_DIVU, rd, rs1, rs2);
}

uint32_t AsmDivw(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG_64, FUNC_MULT, FUNC3_DIV, rd, rs1, rs2);
}

uint32_t AsmRem(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_MULT, FUNC3_REM, rd, rs1, rs2);
}

uint32_t AsmRemu(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG, FUNC_MULT, FUNC3_REMU, rd, rs1, rs2);
}

uint32_t AsmRemw(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG_64, FUNC_MULT, FUNC3_REM, rd, rs1, rs2);
}

uint32_t AsmRemuw(uint32_t rd, uint32_t rs1, uint32_t rs2) {
  return AsmRType(OPCODE_ARITHLOG_64, FUNC_MULT, FUNC3_REMU, rd, rs1, rs2);
}

} // namespace RISCV_EMULATOR