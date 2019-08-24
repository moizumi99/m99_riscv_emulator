#include "RISCV_Emulator.h"
#include "assembler.h"
#include <iostream>

using namespace std;

uint32_t op_code(uint32_t);
uint32_t op_regA(uint32_t);
uint32_t op_regB(uint32_t);
uint32_t op_data(uint32_t);
uint32_t op_addr(uint32_t);

uint32_t reg[8];
uint32_t rom[256];
uint32_t ram[256];

int main() {
  uint32_t pc;
  uint32_t ir;
  uint32_t flag_eq;

  assembler(rom);

  pc = 0;
  flag_eq = 0;

  do {

    ir = rom[pc];
    printf(" %5d  %5x  %5d  %5d  %5d  %5d\n", pc, ir, reg[0], reg[1], reg[2],
           reg[3]);

    pc = pc + 1;

    switch (op_code(ir)) {
    case MOV:
      reg[op_regA(ir)] = reg[op_regB(ir)];
      break;
    case ADD:
      reg[op_regA(ir)] = reg[op_regA(ir)] + reg[op_regB(ir)];
      break;
    case SUB:
      reg[op_regA(ir)] = reg[op_regA(ir)] - reg[op_regB(ir)];
      break;
    case AND:
      reg[op_regA(ir)] = reg[op_regA(ir)] & reg[op_regB(ir)];
      break;
    case OR:
      reg[op_regA(ir)] = reg[op_regA(ir)] | reg[op_regB(ir)];
      break;
    case SL:
      reg[op_regA(ir)] = reg[op_regA(ir)] << 1;
      break;
    case SR:
      reg[op_regA(ir)] = reg[op_regA(ir)] >> 1;
      break;
    case SRA:
      reg[op_regA(ir)] =
          (reg[op_regA(ir)] & 0x8000) | (reg[op_regA(ir)] >> 1);
      break;
    case LDL:
      reg[op_regA(ir)] =
          (reg[op_regA(ir)] & 0xff00) | (op_data(ir) & 0x00ff);
      break;
    case LDH:
      reg[op_regA(ir)] =
          (op_data(ir) << 8) | (reg[op_regA(ir)] & 0x00ff);
      break;
    case CMP:
      if (reg[op_regA(ir)] == reg[op_regB(ir)]) {
        flag_eq = 1;
      } else {
        flag_eq = 0;
      }
      break;
    case JE:
      if (flag_eq == 1)
        pc = op_addr(ir);
      break;
    case JMP:
      pc = op_addr(ir);
      break;
    case LD:
      reg[op_regA(ir)] = ram[op_addr(ir)];
      break;
    case ST:
      ram[op_addr(ir)] = reg[op_regA(ir)];
      break;
    default:
      break;
    }
  } while (op_code(ir) != HLT);

  printf("ram[64] = %d \n", ram[64]);
  return 0;
}

uint32_t op_code(uint32_t ir) { return (ir >> 11); }
uint32_t op_regA(uint32_t ir) { return ((ir >> 8) & 0x0007); }
uint32_t op_regB(uint32_t ir) { return ((ir >> 5) & 0x0007); }
uint32_t op_data(uint32_t ir) { return (ir & 0x00ff); }
uint32_t op_addr(uint32_t ir) { return (ir & 0x00ff); }