#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <cstdint>
uint8_t *add_cmd(uint8_t *mem, uint32_t cmd);

uint32_t asm_add(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_sub(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_and(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_or(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_xor(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_sll(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_srl(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_sra(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_slt(uint32_t rd, uint32_t rs1, uint32_t rs2);

uint32_t asm_addi(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t asm_andi(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t asm_ori(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t asm_xori(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t asm_slli(uint32_t rd, uint32_t rs1, int32_t imm12);

uint32_t asm_beq(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t asm_bge(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t asm_bltu(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t asm_bne(uint32_t rs1, uint32_t rs2, int32_t offset13);

uint32_t asm_jal(uint32_t rd, int32_t offset21);

uint32_t asm_lw(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t asm_sw(uint32_t rs1, uint32_t rs2, int32_t offset12);

uint32_t asm_jalr(uint32_t rd, uint32_t rs1, int32_t offset12);

uint32_t asm_lui(uint32_t rd, int32_t imm20);

#endif // ASSEMBLER_H