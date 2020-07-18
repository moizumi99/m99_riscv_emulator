#include "load_assembler.h"
#include "RISCV_cpu.h"
#include "instruction_encdec.h"
#include <cstdint>
#include <cassert>

namespace CPU_TEST {

// R_TYPE
uint32_t
AsmRType(op_label opcode, op_funct funct, op_funct3 funct3, uint32_t rd,
         uint32_t rs1, uint32_t rs2) {
  RType cmd;
  cmd.opcode = opcode;
  cmd.funct7 = funct;
  cmd.funct3 = funct3;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.rs2 = rs2;
  return cmd.GetValue();
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

// A_TYPE (a variation of R-Type)
uint32_t
AsmAType(op_label opcode, op_funct5 funct5, op_funct3 funct3, uint32_t rd,
         uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  RType cmd;
  uint32_t funct7 = (funct5 << 2) | ((aq & 0b1) << 1) | (rl & 0b1);
  cmd.opcode = opcode;
  cmd.funct7 = funct7;
  cmd.funct3 = funct3;
  cmd.rd = rd;
  cmd.rs1 = rs1;
  cmd.rs2 = rs2;
  return cmd.GetValue();
}

uint32_t AsmAmoAddd(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOADD, FUNC3_AMOD, rd, rs1, rs2, aq, rl);
}

uint32_t AsmAmoAddw(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOADD, FUNC3_AMOW, rd, rs1, rs2, aq, rl);
}

uint32_t AsmAmoAndd(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOAND, FUNC3_AMOD, rd, rs1, rs2, aq, rl);
}
uint32_t AsmAmoAndw(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOAND, FUNC3_AMOW, rd, rs1, rs2, aq, rl);
}

uint32_t AsmAmoMaxd(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOMAX, FUNC3_AMOD, rd, rs1, rs2, aq, rl);
}
uint32_t AsmAmoMaxw(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOMAX, FUNC3_AMOW, rd, rs1, rs2, aq, rl);
}

uint32_t AsmAmoMaxud(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOMAXU, FUNC3_AMOD, rd, rs1, rs2, aq, rl);
}
uint32_t AsmAmoMaxuw(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOMAXU, FUNC3_AMOW, rd, rs1, rs2, aq, rl);
}

uint32_t AsmAmoMind(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOMIN, FUNC3_AMOD, rd, rs1, rs2, aq, rl);
}
uint32_t AsmAmoMinw(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOMIN, FUNC3_AMOW, rd, rs1, rs2, aq, rl);
}

uint32_t AsmAmoMinud(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOMINU, FUNC3_AMOD, rd, rs1, rs2, aq, rl);
}
uint32_t AsmAmoMinuw(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOMINU, FUNC3_AMOW, rd, rs1, rs2, aq, rl);
}

uint32_t AsmAmoOrd(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOOR, FUNC3_AMOD, rd, rs1, rs2, aq, rl);
}
uint32_t AsmAmoOrw(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOOR, FUNC3_AMOW, rd, rs1, rs2, aq, rl);
}

uint32_t AsmAmoSwapd(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOSWAP, FUNC3_AMOD, rd, rs1, rs2, aq, rl);
}
uint32_t AsmAmoSwapw(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOSWAP, FUNC3_AMOW, rd, rs1, rs2, aq, rl);
}

uint32_t AsmAmoXord(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOXOR, FUNC3_AMOD, rd, rs1, rs2, aq, rl);
}
uint32_t AsmAmoXorw(uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t aq, uint32_t rl) {
  return AsmAType(OPCODE_AMO, FUNC5_AMOXOR, FUNC3_AMOW, rd, rs1, rs2, aq, rl);
}

// I_TYPE
uint32_t AsmIType(op_label opcode, op_funct3 funct3, uint32_t rd, uint32_t rs1,
                  int32_t imm12) {
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
uint32_t AsmBType(op_label opcode, op_funct3 funct3, uint32_t rs1, uint32_t rs2,
                  int32_t offset13) {
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
uint32_t AsmSType(op_label opcode, op_funct3 funct3, uint32_t rs1, uint32_t rs2,
                  int32_t offset12) {
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

uint16_t AsmCAddType(uint16_t cmd, uint32_t rd, uint32_t rs2) {
  cmd |= ((rd & 0x1F) << 7) | ((rs2 & 0x1F) << 2);
  return cmd;
}

uint16_t AsmCAdd(uint32_t rd, uint32_t rs2) {
  uint16_t cmd = 0b1001000000000010;
  return AsmCAddType(cmd, rd, rs2);
}

uint16_t AsmCEbreak() {
  uint16_t cmd = 0b1001000000000010;
  return cmd;
}

uint16_t AsmCFldsp(uint32_t rd, uint32_t uimm) {
  uint16_t cmd = 0b0010000000000010;
  cmd |= (rd & 0x1F) << 7;
  cmd |= ((uimm >> 5) & 1) << 12;
  cmd |= ((uimm >> 3) & 0b11) << 5;
  cmd |= ((uimm >> 6) & 0b111) << 2;
  return cmd;
}

uint16_t AsmCFlwsp(uint32_t rd, uint32_t uimm) {
  uint16_t cmd = 0b0110000000000010;
  cmd |= (rd & 0x1F) << 7;
  cmd |= ((uimm >> 5) & 1) << 12;
  cmd |= ((uimm >> 2) & 0b111) << 4;
  cmd |= ((uimm >> 6) & 0b11) << 2;
  return cmd;
}

uint16_t AsmCFsdsp(uint32_t rs2, uint32_t uimm) {
  uint16_t cmd = 0b1010000000000010;
  cmd |= (rs2 & 0x1F) << 2;
  cmd |= ((uimm >> 3) & 0b111) << 10;
  cmd |= ((uimm >> 6) & 0b111) << 7;
  return cmd;
}

uint16_t AsmCFswsp(uint32_t rs2, uint32_t uimm) {
  uint16_t cmd = 0b1110000000000010;
  cmd |= (rs2 & 0x1F) << 2;
  cmd |= ((uimm >> 2) & 0b1111) << 9;
  cmd |= ((uimm >> 6) & 0b11) << 7;
  return cmd;
}

uint16_t AsmCJalr(uint32_t rs1) {
  uint16_t cmd = 0b1001000000000010;
  cmd |= (rs1 & 0b011111) << 7;
  return cmd;
}

uint16_t AsmCJr(uint32_t rs1) {
  uint16_t cmd = 0b1000000000000010;
  cmd |= (rs1 & 0b011111) << 7;
  return cmd;
}

uint16_t AsmCLdsp(uint32_t rd, uint32_t uimm) {
  uint16_t cmd = 0b0110000000000010;
  cmd |= (rd & 0x1F) << 7;
  cmd |= ((uimm >> 5) & 1) << 12;
  cmd |= ((uimm >> 3) & 0b11) << 5;
  cmd |= ((uimm >> 6) & 0b111) << 2;
  return cmd;
}

uint16_t AsmCLwsp(uint32_t rd, uint32_t uimm) {
  uint16_t cmd = 0b0100000000000010;
  cmd |= (rd & 0x1F) << 7;
  cmd |= ((uimm >> 5) & 1) << 12;
  cmd |= ((uimm >> 2) & 0b111) << 4;
  cmd |= ((uimm >> 6) & 0b11) << 2;
  return cmd;
}

uint16_t AsmCMv(uint32_t rd, uint32_t rs2) {
  uint16_t cmd = 0b01000000000000010;
  return AsmCAddType(cmd, rd, rs2);
}

uint16_t AsmCSdsp(uint32_t rs2, uint32_t uimm) {
  uint16_t cmd = 0b1110000000000010;
  cmd |= (rs2 & 0x1F) << 2;
  cmd |= ((uimm >> 3) & 0b111) << 10;
  cmd |= ((uimm >> 6) & 0b111) << 7;
  return cmd;
}

uint16_t AsmCSlli(uint32_t rd, uint32_t uimm) {
  uint16_t cmd = 0b0000000000000010;
  cmd |= (rd & 0x1F) << 7;
  cmd |= ((uimm >> 5) & 1) << 12;
  cmd |= (uimm & 0b11111) << 2;
  return cmd;
}

uint16_t AsmCSwsp(uint32_t rs2, uint32_t uimm) {
  uint16_t cmd = 0b1100000000000010;
  cmd |= (rs2 & 0x1F) << 2;
  cmd |= ((uimm >> 2) & 0b1111) << 9;
  cmd |= ((uimm >> 6) & 0b11) << 7;
  return cmd;
}

uint16_t AsmCAddiType(uint16_t cmd, uint32_t rd, int32_t imm) {
  cmd |= (rd & 0x1F) << 7;
  cmd |= ((imm >> 5) & 1) << 12;
  cmd |= (imm & 0b11111) << 2;
  return cmd;
}

uint16_t AsmCAddi(uint32_t rd, int32_t imm) {
  uint16_t cmd = 0b0000000000000001;
  return AsmCAddiType(cmd, rd, imm);
}

uint16_t AsmCLi(uint32_t rd, int32_t imm) {
  uint16_t cmd = 0b0100000000000001;
  return AsmCAddiType(cmd, rd, imm);
}

uint16_t AsmCAddi16sp(int32_t imm) {
  uint16_t cmd = 0b0110000100000001;
  cmd |= ((imm >> 9) & 1) << 12;
  cmd |= ((imm >> 5) & 1) << 2;
  cmd |= ((imm >> 7) & 0b11) << 3;
  cmd |= ((imm >> 6) & 1) << 5;
  cmd |= ((imm >> 4) & 1) << 6;
  return cmd;
}

uint16_t AsmCAddiw(uint32_t rd, int32_t imm) {
  uint16_t cmd = 0b0010000000000001;
  cmd |= (rd & 0x1F) << 7;
  cmd |= ((imm >> 5) & 1) << 12;
  cmd |= (imm & 0b11111) << 2;
  return cmd;
}

uint16_t AsmCAndType(uint16_t cmd, uint32_t rd, uint32_t rs2) {
  cmd |= (rd & 0b111) << 7;
  cmd |= (rs2 & 0b111) << 2;
  return cmd;
}

uint16_t AsmCAnd(uint32_t rd, uint32_t rs2) {
  uint16_t cmd = 0b1000110001100001;
  return AsmCAndType(cmd, rd, rs2);
}

uint16_t AsmCAddw(uint32_t rd, uint32_t rs2) {
  uint16_t cmd = 0b1001110000100001;
  return AsmCAndType(cmd, rd, rs2);
}

uint16_t AsmCOr(uint32_t rd, uint32_t rs2) {
  uint16_t cmd = 0b1000110001000001;
  return AsmCAndType(cmd, rd, rs2);
}

uint16_t AsmCSub(uint32_t rd, uint32_t rs2) {
  uint16_t cmd = 0b1000110000000001;
  return AsmCAndType(cmd, rd, rs2);
}

uint16_t AsmCSubw(uint32_t rd, uint32_t rs2) {
  uint16_t cmd = 0b1001110000000001;
  return AsmCAndType(cmd, rd, rs2);
}

uint16_t AsmCXor(uint32_t rd, uint32_t rs2) {
  uint16_t cmd = 0b1000110000100001;
  return AsmCAndType(cmd, rd, rs2);
}

uint16_t AsmCAndiType(uint16_t cmd, uint32_t rd, int32_t imm) {
  cmd |= (rd & 0b111) << 7;
  cmd |= ((imm >> 5) & 1) << 12;
  cmd |= (imm & 0b11111) << 2;
  return cmd;
}

uint16_t AsmCAndi(uint32_t rd, int32_t imm) {
  uint16_t cmd = 0b1000100000000001;
  return AsmCAndiType(cmd, rd, imm);
}

uint16_t AsmCSrai(uint32_t rd, int32_t imm) {
  uint16_t cmd = 0b1000010000000001;
  return AsmCAndiType(cmd, rd, imm);
}

uint16_t AsmCSrli(uint32_t rd, int32_t imm) {
  uint16_t cmd = 0b1000000000000001;
  return AsmCAndiType(cmd, rd, imm);
}

uint16_t AsmCBType(uint16_t  cmd, uint32_t rs1, int32_t offset) {
  cmd |= (rs1 & 0b111) << 7;
  cmd |= ((offset >> 8) & 1) << 12;
  cmd |= ((offset >> 3) & 0b11) << 10;
  cmd |= ((offset >> 6) & 0b11) << 5;
  cmd |= ((offset >> 1) & 0b11) << 3;
  cmd |= ((offset >> 5) & 1) << 2;
  return cmd;
}

uint16_t AsmCBeqz(uint32_t rs1, int32_t offset) {
  uint16_t cmd = 0b1100000000000001;
  return AsmCBType(cmd, rs1, offset);
}

uint16_t AsmCBnez(uint32_t rs1, int32_t offset) {
  uint16_t cmd = 0b1110000000000001;
  return AsmCBType(cmd, rs1, offset);
}

uint16_t AsmCJType(uint16_t cmd, int32_t offset) {
  cmd |= ((offset >> 11) & 1) << 12;
  cmd |= ((offset >> 4) & 1) << 11;
  cmd |= ((offset >> 8) & 0b11) << 9;
  cmd |= ((offset >> 10) & 1) << 8;
  cmd |= ((offset >> 6) & 1) << 7;
  cmd |= ((offset >> 7) & 1) << 6;
  cmd |= ((offset >> 1) & 0b111) << 3;
  cmd |= ((offset >> 5) & 1) << 2;
  return cmd;
}

uint16_t AsmCJ(int32_t offset) {
  uint16_t cmd = 0b1010000000000001;
  return AsmCJType(cmd, offset);
}

uint16_t AsmCJal(int32_t offset) {
  uint16_t cmd = 0b0010000000000001;
  return AsmCJType(cmd, offset);
}

uint16_t AsmCLui(uint32_t rd, int32_t imm) {
  uint16_t cmd = 0b0110000000000001;
  cmd |= ((imm >> 17) & 1) << 12;
  cmd |= ((imm >> 12) & 0b11111) << 2;
  return cmd;
}

uint16_t AsmCAddi4spn(uint32_t rd, uint32_t uimm) {
  uint16_t cmd = 0b0000000000000000;
  cmd |= ((rd & 0b111) << 2);
  cmd |= ((uimm >> 4) & 0b11) << 11;
  cmd |= ((uimm >> 6) & 0b1111) << 7;
  cmd |= ((uimm >> 2) & 0b1) << 6;
  cmd |= ((uimm >> 3) & 0b1) << 5;
  return cmd;
}

uint16_t AsmCLdType(uint16_t cmd, uint32_t r4to2, uint32_t r9to7, uint32_t uimm) {
  cmd |= ((r4to2 & 0b111) << 2);
  cmd |= ((r9to7 & 0b111) << 7);
  cmd |= ((uimm >> 3) & 0b111) << 10;
  cmd |= ((uimm >> 6) & 0b11) << 5;
  return cmd;
}

uint16_t AsmCLwType(uint16_t cmd, uint32_t r4to2, uint32_t r9to7, uint32_t uimm) {
  cmd |= ((r4to2 & 0b111) << 2);
  cmd |= ((r9to7 & 0b111) << 7);
  cmd |= ((uimm >> 3) & 0b111) << 10;
  cmd |= ((uimm >> 2) & 0b1) << 6;
  cmd |= ((uimm >> 6) & 0b1) << 5;
  return cmd;
}

uint16_t AsmCFld(uint32_t rd, uint32_t rs1, uint32_t uimm) {
  uint16_t cmd = 0b0010000000000000;
  return AsmCLdType(cmd, rd, rs1, uimm);
}

uint16_t AsmCFlw(uint32_t rd, uint32_t rs1, uint32_t uimm) {
  uint16_t cmd = 0b0110000000000000;
  return AsmCLwType(cmd, rd, rs1, uimm);
}

uint16_t AsmCFsd(uint32_t rs1, uint32_t rs2, uint32_t uimm) {
  uint16_t cmd = 0b01010000000000000;
  return AsmCLdType(cmd, rs2, rs1, uimm);
}

uint16_t AsmCFsw(uint32_t rs1, uint32_t rs2, uint32_t uimm) {
  uint16_t cmd = 0b1110000000000000;
  return AsmCLwType(cmd, rs2, rs1, uimm);
}

uint16_t AsmCLd(uint32_t rd, uint32_t rs1, uint32_t uimm) {
  uint16_t cmd = 0b0110000000000000;
  return AsmCLdType(cmd, rd, rs1, uimm);
}

uint16_t AsmCLw(uint32_t rd, uint32_t rs1, uint32_t uimm) {
  uint16_t cmd = 0b0100000000000000;
  return AsmCLwType(cmd, rd, rs1, uimm);
}

uint16_t AsmCSd(uint32_t rs1, uint32_t rs2, uint32_t uimm) {
  uint16_t cmd = 0b01110000000000000;
  return AsmCLdType(cmd, rs2, rs1, uimm);
}

uint16_t AsmCsw(uint32_t rs1, uint32_t rs2, uint32_t uimm) {
  uint16_t cmd = 0b1100000000000000;
  return AsmCLwType(cmd, rs2, rs1, uimm);
}

} // namespace RISCV_EMULATOR