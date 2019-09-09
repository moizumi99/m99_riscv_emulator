#include "RISCV_cpu.h"
#include "bit_tools.h"
#include "instruction_encdec.h"
#include <iostream>
#include <stdint.h>

using namespace std;

uint32_t get_code(uint32_t ir);

uint32_t reg[32];
uint32_t pc;

void set_register(uint32_t num, uint32_t value) { reg[num] = value; }

uint32_t read_register(uint32_t num) { return reg[num]; }

uint32_t load_cmd(uint8_t *mem, uint32_t pc) {
  uint8_t *address = mem + pc;
  return *address | (*(address + 1) << 8) | (*(address + 2) << 16) |
         (*(address + 3) << 24);
}

int run_cpu(uint8_t *mem, uint32_t start_pc, bool verbose) {
  bool error_flag = false;
  bool end_flag = false;

  if (verbose) {
    printf("   PC    Binary       T0       T1       T2       T3       A0       "
           "A1       A2       A3       A4       A5       A6       A7\n");
  }

  pc = start_pc;
  do {
    uint32_t next_pc;
    uint32_t ir;

    // TODO: memory is byte aligned. Fix this.
    ir = load_cmd(mem, pc);
    if (verbose) {
      printf(" %4x  %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x "
             "%08x %08x\n",
             pc, ir, reg[T0], reg[T1], reg[T2], reg[T3], reg[A0], reg[A1],
             reg[A2], reg[A3], reg[A4], reg[A5], reg[A6], reg[A7]);
    }

    next_pc = pc + 4;

    // Decode. Mimick the HW behavior. (In HW, decode is in parallel.)
    uint8_t instruction = get_code(ir);
    uint8_t rd = get_rd(ir);
    uint8_t rs1 = get_rs1(ir);
    uint8_t rs2 = get_rs2(ir);
    int16_t imm12 = get_imm12(ir);
    uint8_t shamt = get_shamt(ir);
    int16_t imm13 = get_imm13(ir);
    int32_t imm21 = get_imm21(ir);
    int16_t imm12_stype = get_stype_imm12(ir);
    uint32_t address;
    int32_t sreg_rs1, sreg_rs2;

    switch (instruction) {
    case INST_ADD:
      reg[rd] = reg[rs1] + reg[rs2];
      break;
    case INST_ADDI:
      reg[rd] = reg[rs1] + imm12;
      break;
    case INST_SLLI:
      reg[rd] = reg[rs1] << shamt;
      // With RIV32I, shamt[5] must be zero.
      error_flag |= (shamt >> 4) & 1;
      break;
    case INST_BEQ:
      if (reg[rs1] == reg[rs2]) {
        next_pc = pc + imm13;
      }
      break;
    case INST_BGE:
      sreg_rs1 = static_cast<int32_t>(reg[rs1]);
      sreg_rs2 = static_cast<int32_t>(reg[rs2]);
      if (sreg_rs1 >= sreg_rs2) {
        next_pc = pc + imm13;
      }
      break;
    case INST_BLTU:
      if (reg[rs1] < reg[rs2]) {
        next_pc = pc + imm13;
      }
      break;
    case INST_BNE:
      if (reg[rs1] != reg[rs2]) {
        next_pc = pc + imm13;
      }
      break;
    case INST_JAL:
      reg[rd] = pc + 4;
      next_pc = pc + imm21;
      break;
    case INST_JALR:
      next_pc = pc + reg[rs1] + imm12;
      reg[rd] = pc + 4;
      if (rd == ZERO && rs1 == RA && imm12 == 0) {
        end_flag = true;
      }
      break;
    case INST_LW:
      address = reg[rs1] + imm12;
      reg[rd] = load_wd(mem + address);
      break;
    case INST_SW:
      address = reg[rs1] + imm12_stype;
      store_wd(mem + address, reg[rs2]);
      break;
    default:
      error_flag = true;
      break;
    }
    reg[ZERO] = 0;

    pc = next_pc & 0xFFFF;
  } while (!error_flag && !end_flag);

  return error_flag;
}

uint32_t get_code(uint32_t ir) {
  uint16_t opcode = bitcrop(ir, 7, 0);
  uint8_t funct3 = bitcrop(ir, 3, 12);
  // uint8_t funct7 = bitcrop(ir, 7, 25);
  uint32_t instruction = 0;
  switch (opcode) {
  case OPCODE_ADD: // ADD, SUB
    if (funct3 == FUNC3_ADD) {
      instruction = INST_ADD;
    } else if (funct3 == FUNC3_SUB) {
      instruction = INST_SUB;
    }
    break;
  case OPCODE_ADDI: // ADDI, SUBI
    if (funct3 == FUNC3_ADDI) {
      instruction = INST_ADDI;
    } else if (funct3 == FUNC3_SLLI) {
      instruction = INST_SLLI;
    }
    break;
  case OPCODE_B: // beq, bltu, bge, bne
    if (funct3 == FUNC3_BEQ) {
      instruction = INST_BEQ;
    } else if (funct3 == FUNC3_BLTU) {
      instruction = INST_BLTU;
    } else if (funct3 == FUNC3_BGE) {
      instruction = INST_BGE;
    } else if (funct3 == FUNC3_BNE) {
      instruction = INST_BNE;
    }
    break;
  case OPCODE_J: // jal
    instruction = INST_JAL;
    break;
  case OPCODE_JALR: // jalr
    if (funct3 == FUNC3_JALR) {
      instruction = INST_JALR;
    }
    break;
  case OPCODE_LD: // LW
    if (funct3 == FUNC3_LW) {
      instruction = INST_LW;
    }
    break;
  case OPCODE_S: // SW
    if (funct3 == FUNC3_SW) {
      instruction = INST_SW;
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
