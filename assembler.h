#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "memory_wrapper.h"
#include <cstdint>
#include <vector>

using namespace RISCV_EMULATOR;

namespace CPU_TEST {

MemorWrapperIterator AddCmd(MemorWrapperIterator &mem, uint32_t cmd);

uint32_t AsmAdd(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmAddw(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmSub(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmSubw(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmAnd(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmOr(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmXor(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmSll(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmSllw(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmSrl(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmSrlw(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmSra(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmSraw(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmSlt(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmSltu(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmMret();

uint32_t AsmAddi(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t AsmAddiw(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t AsmAndi(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t AsmOri(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t AsmXori(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t AsmSlli(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t AsmSlliw(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t AsmSrli(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t AsmSrliw(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t AsmSrai(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t AsmSraiw(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t AsmSlti(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t AsmSltiu(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t AsmBeq(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t AsmBge(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t AsmBgeu(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t AsmBlt(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t AsmBltu(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t AsmBne(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t AsmJal(uint32_t rd, int32_t offset21);

uint32_t AsmLb(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t AsmLbu(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t AsmLh(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t AsmLhu(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t AsmLw(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t AsmLd(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t AsmLwu(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t AsmSw(uint32_t rs1, uint32_t rs2, int32_t offset12);

uint32_t AsmSh(uint32_t rs1, uint32_t rs2, int32_t offset12);

uint32_t AsmSb(uint32_t rs1, uint32_t rs2, int32_t offset12);

uint32_t AsmSd(uint32_t rs1, uint32_t rs2, int32_t offset12);

uint32_t AsmJalr(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t AsmEbreak();

uint32_t AsmEcall();

uint32_t AsmCsrrc(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t AsmCsrrci(uint32_t rd, uint32_t zimm, int32_t offset12);

uint32_t AsmCsrrs(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t AsmCsrrsi(uint32_t rd, uint32_t zimm, int32_t offset12);

uint32_t AsmCsrrw(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t AsmCsrrwi(uint32_t rd, uint32_t zimm, int32_t offset12);

uint32_t AsmFence(uint32_t pred, uint32_t succ);

uint32_t AsmFencei();

uint32_t AsmLui(uint32_t rd, int32_t imm20);

uint32_t AsmAuipc(uint32_t rd, int32_t imm20);

uint32_t AsmMul(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmMulh(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmMulhsu(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmMulhu(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmMulw(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmDiv(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmDivu(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmDivuw(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmDivw(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmRem(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmRemu(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmRemw(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t AsmRemuw(uint32_t rd, uint32_t rs1, uint32_t rs2);

} // namespace RISCV_EMULATOR

#endif // ASSEMBLER_H