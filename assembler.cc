#include "load_assembler.h"
#include "RISCV_cpu.h"
#include "instruction_encdec.h"
#include <cstdint>

// R_TYPE
uint32_t asm_add(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    r_type cmd;
    cmd.funct7 = FUNC_NORM;
    cmd.opcode = OPCODE_ARITHLOG;
    cmd.rs2 = rs2;
    cmd.rs1 = rs1;
    cmd.rd = rd;
    cmd.funct3 = FUNC3_ADDSUB;
    return cmd.value();
}

uint32_t asm_sub(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    r_type cmd;
    cmd.funct7 = FUNC_ALT;
    cmd.opcode = OPCODE_ARITHLOG;
    cmd.rs2 = rs2;
    cmd.rs1 = rs1;
    cmd.rd = rd;
    cmd.funct3 = FUNC3_ADDSUB;
    return cmd.value();
}

uint32_t asm_and(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    r_type cmd;
    cmd.funct7 = FUNC_NORM;
    cmd.opcode = OPCODE_ARITHLOG;
    cmd.rs2 = rs2;
    cmd.rs1 = rs1;
    cmd.rd = rd;
    cmd.funct3 = FUNC3_AND;
    return cmd.value();
}

uint32_t asm_or(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    r_type cmd;
    cmd.funct7 = FUNC_NORM;
    cmd.opcode = OPCODE_ARITHLOG;
    cmd.rs2 = rs2;
    cmd.rs1 = rs1;
    cmd.rd = rd;
    cmd.funct3 = FUNC3_OR;
    return cmd.value();
}

uint32_t asm_xor(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    r_type cmd;
    cmd.funct7 = FUNC_NORM;
    cmd.opcode = OPCODE_ARITHLOG;
    cmd.rs2 = rs2;
    cmd.rs1 = rs1;
    cmd.rd = rd;
    cmd.funct3 = FUNC3_XOR;
    return cmd.value();
}

uint32_t asm_sll(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    r_type cmd;
    cmd.funct7 = FUNC_NORM;
    cmd.opcode = OPCODE_ARITHLOG;
    cmd.rs2 = rs2;
    cmd.rs1 = rs1;
    cmd.rd = rd;
    cmd.funct3 = FUNC3_SL;
    return cmd.value();
}

uint32_t asm_srl(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    r_type cmd;
    cmd.funct7 = FUNC_NORM;
    cmd.opcode = OPCODE_ARITHLOG;
    cmd.rs2 = rs2;
    cmd.rs1 = rs1;
    cmd.rd = rd;
    cmd.funct3 = FUNC3_SR;
    return cmd.value();
}

uint32_t asm_sra(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    r_type cmd;
    cmd.funct7 = FUNC_ALT;
    cmd.opcode = OPCODE_ARITHLOG;
    cmd.rs2 = rs2;
    cmd.rs1 = rs1;
    cmd.rd = rd;
    cmd.funct3 = FUNC3_SR;
    return cmd.value();
}

// I_TYPE
uint32_t asm_addi(uint32_t rd, uint32_t rs1, int32_t imm12) {
    i_type cmd;
    cmd.imm12 = imm12 & 0xFFF;
    cmd.rd = rd;
    cmd.rs1 = rs1;
    cmd.opcode = OPCODE_ADDI;
    cmd.funct3 = FUNC3_ADDSUB;
    return cmd.value();
}

uint32_t asm_slli(uint32_t rd, uint32_t rs1, int32_t imm12) {
    i_type cmd;
    // SLLI immediate is 6 bit wide.
    cmd.imm12 = imm12 & 0b0111111;
    cmd.rd = rd;
    cmd.rs1 = rs1;
    cmd.opcode = OPCODE_ADDI;
    cmd.funct3 = FUNC3_SL;
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
    cmd.funct3 = FUNC3_LS;
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
    cmd.funct3 = FUNC3_LS;
    cmd.rs2 = rs2;
    cmd.rs1 = rs1;
    cmd.imm12 = offset12;
    return cmd.value();
}

// U TYPE
uint32_t asm_lui(uint32_t rd, int32_t imm20) {
    u_type cmd;
    cmd.opcode = OPCODE_LUI;
    cmd.rd = rd;
    cmd.imm20 = imm20 & 0x0FFFFF;
    return cmd.value();
}

