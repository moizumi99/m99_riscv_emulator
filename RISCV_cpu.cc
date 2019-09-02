#include "RISCV_cpu.h"
#include "bit_tools.h"
#include "instruction_encdec.h"
#include <iostream>
#include <stdint.h>

using namespace std;

uint32_t get_code(uint32_t ir);

uint32_t reg[32];

int run_cpu(uint32_t *rom) {
  uint32_t pc, next_pc;
  uint32_t ir;
  bool error_flag = false;
  bool end_flag = false;

  pc = 0;

  do {

    // TODO: memory is byte aligned. Fix this.
    ir = rom[pc / 4];
    printf(" %4d  %08x  %5d  %5d  %5d  %5d %5d\n", pc, ir, reg[T0], reg[T1],
           reg[T2], reg[T3], reg[A0]);

    next_pc = pc + 4;

    uint32_t rd = get_rd(ir);
    switch (get_code(ir)) {
    case INST_ADD:
      reg[rd] = reg[get_rs1(ir)] + reg[get_rs2(ir)];
      // TODO: update flag
      break;
    case INST_ADDI:
      reg[rd] = reg[get_rs1(ir)] + get_imm12(ir);
      // TODO: update flag
      break;
    case INST_BEQ:
      if (reg[get_rs1(ir)] == reg[get_rs2(ir)]) {
        next_pc = pc + get_imm13(ir);
      }
      break;
    case INST_JAL:
      reg[get_rd(ir)] = pc + 4;
      next_pc = pc + get_imm21(ir);
      break;
    case INST_JALR:
      next_pc = pc + reg[get_rs1(ir)] + get_imm12(ir);
      reg[get_rd(ir)] = pc + 4;
      if (get_rd(ir) == ZERO && get_rs1(ir) == RA && get_imm12(ir) == 0) {
        end_flag = true;
      }
      break;
    default:
      error_flag = true;
      break;
    }
    reg[ZERO] = 0;

    pc = next_pc & 0xFFFF;
  } while (!error_flag && !end_flag);

  return reg[A0];
}

uint32_t get_code(uint32_t ir) {
  uint16_t opcode = bitcrop(ir, 7, 0);
  uint8_t funct3 = bitcrop(ir, 3, 12);
  // uint8_t funct7 = bitcrop(ir, 7, 25);
  uint32_t instruction = 0;
  switch (opcode) {
  case 0b0110011: // ADD, SUB
    if (funct3 == FUNC3_ADD) {
      instruction = INST_ADD;
    } else if (funct3 == FUNC3_SUB) {
      instruction = INST_SUB;
    }
    break;
  case 0b0010011: // ADDI, SUBI
    if (funct3 == FUNC3_ADDI) {
      instruction = INST_ADDI;
    }
    break;
  case 0b1100011: // beq
    if (funct3 == FUNC3_BEQ) {
      instruction = INST_BEQ;
    }
    break;
  case 0b1101111: // jal
    instruction = INST_JAL;
    break;
  case 0b1100111: // jalr
    if (funct3 == FUNC3_JALR) {
      instruction = INST_JALR;
    }
    break;
  default:
    break;
  }
  if (instruction == 0) {
    printf("Error decoding 0x%08x\n", ir);
  }
  return instruction;
}
