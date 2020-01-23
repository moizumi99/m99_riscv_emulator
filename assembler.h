#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "memory_wrapper.h"
#include <cstdint>
#include <vector>

memory_wrapper_iterator add_cmd(memory_wrapper_iterator &mem, uint32_t cmd);

uint32_t asm_add(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_sub(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_and(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_or(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_xor(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_sll(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_srl(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_sra(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_slt(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_sltu(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_mret();

uint32_t asm_addi(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t asm_andi(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t asm_ori(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t asm_xori(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t asm_slli(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t asm_srli(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t asm_srai(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t asm_slti(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t asm_sltiu(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t asm_beq(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t asm_bge(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t asm_bgeu(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t asm_blt(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t asm_bltu(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t asm_bne(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t asm_jal(uint32_t rd, int32_t offset21);

uint32_t asm_lb(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t asm_lbu(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t asm_lh(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t asm_lhu(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t asm_lw(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t asm_sw(uint32_t rs1, uint32_t rs2, int32_t offset12);

uint32_t asm_sh(uint32_t rs1, uint32_t rs2, int32_t offset12);

uint32_t asm_sb(uint32_t rs1, uint32_t rs2, int32_t offset12);

uint32_t asm_jalr(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t asm_ebreak();

uint32_t asm_ecall();

uint32_t asm_csrrc(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t asm_csrrci(uint32_t rd, uint32_t zimm, int32_t offset12);

uint32_t asm_csrrs(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t asm_csrrsi(uint32_t rd, uint32_t zimm, int32_t offset12);

uint32_t asm_csrrw(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t asm_csrrwi(uint32_t rd, uint32_t zimm, int32_t offset12);

uint32_t asm_fence(uint32_t pred, uint32_t succ);

uint32_t asm_fencei();

uint32_t asm_lui(uint32_t rd, int32_t imm20);

uint32_t  asm_auipc(uint32_t rd, int32_t imm20);

#endif // ASSEMBLER_H