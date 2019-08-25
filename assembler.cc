#include "assembler.h"
#include "RISCV_Emulator.h"
#include "RISCV_cpu.h"

uint32_t asm_mov(uint32_t, uint32_t);
uint32_t asm_add(uint32_t, uint32_t);
uint32_t asm_sub(uint32_t, uint32_t);
uint32_t asm_and(uint32_t, uint32_t);
uint32_t asm_or(uint32_t, uint32_t);
uint32_t asm_sl(uint32_t);
uint32_t asm_sr(uint32_t);
uint32_t asm_sra(uint32_t);
uint32_t asm_ldl(uint32_t, uint32_t);
uint32_t asm_ldh(uint32_t, uint32_t);
uint32_t asm_cmp(uint32_t, uint32_t);
uint32_t asm_je(uint32_t);
uint32_t asm_jmp(uint32_t);
uint32_t asm_ld(uint32_t, uint32_t);
uint32_t asm_st(uint32_t, uint32_t);
uint32_t asm_hlt(void);

enum Registers {
  REG0 = 0,
  REG1 = 1,
  REG2 = 2,
  REG3 = 3,
  REG4 = 4,
  REG5 = 5,
  REG6 = 6,
  REG7 = 7
};


void assembler(uint32_t *rom) {
  rom[0] = asm_ldh(REG0, 0);
  rom[1] = asm_ldl(REG0, 0);
  rom[2] = asm_ldh(REG1, 0);
  rom[3] = asm_ldl(REG1, 1);
  rom[4] = asm_ldh(REG2, 0);
  rom[5] = asm_ldl(REG2, 0);
  rom[6] = asm_ldh(REG3, 0);
  rom[7] = asm_ldl(REG3, 10);
  rom[8] = asm_add(REG2, REG1);
  rom[9] = asm_add(REG0, REG2);
  rom[10] = asm_st(REG0, 64);
  rom[11] = asm_cmp(REG2, REG3);
  rom[12] = asm_je(14);
  rom[13] = asm_jmp(8);
  rom[14] = asm_hlt();
}

uint32_t asm_mov(uint32_t ra, uint32_t rb) {
  return ((MOV << 11) | (ra << 8) | (rb << 5));
}

uint32_t asm_add(uint32_t ra, uint32_t rb) {
  return ((ADD << 11) | (ra << 8) | (rb << 5));
}

uint32_t asm_sub(uint32_t ra, uint32_t rb) {
  return ((SUB << 11) | (ra << 8) | (rb << 5));
}

uint32_t asm_and(uint32_t ra, uint32_t rb) {
  return ((AND << 11) | (ra << 8) | (rb << 5));
}

uint32_t asm_or(uint32_t ra, uint32_t rb) {
  return ((OR << 11) | (ra << 8) | (rb << 5));
}

uint32_t asm_sl(uint32_t ra) { return ((SL << 11) | (ra << 8)); }

uint32_t asm_sr(uint32_t ra) { return ((SR << 11) | (ra << 8)); }

uint32_t asm_sra(uint32_t ra) { return ((SRA << 11) | (ra << 8)); }

uint32_t asm_ldl(uint32_t ra, uint32_t ival) {
  return ((LDL << 11) | (ra << 8) | (ival & 0x00ff));
}

uint32_t asm_ldh(uint32_t ra, uint32_t ival) {
  return ((LDH << 11) | (ra << 8) | (ival & 0x00ff));
}

uint32_t asm_cmp(uint32_t ra, uint32_t rb) {
  return ((CMP << 11) | (ra << 8) | (rb << 5));
}

uint32_t asm_je(uint32_t addr) { return ((JE << 11) | (addr & 0x00ff)); }

uint32_t asm_jmp(uint32_t addr) { return ((JMP << 11) | (addr & 0x00ff)); }

uint32_t asm_ld(uint32_t ra, uint32_t addr) {
  return ((LD << 11) | (ra << 8) | (addr & 0x00ff));
}

uint32_t asm_st(uint32_t ra, uint32_t addr) {
  return ((ST << 11) | (ra << 8) | (addr & 0x00ff));
}

uint32_t asm_hlt(void) { return (HLT << 11); }

