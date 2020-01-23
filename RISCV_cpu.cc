#include "RISCV_cpu.h"
#include "bit_tools.h"
#include "instruction_encdec.h"
#include "memory_wrapper.h"
#include <iostream>
#include <tuple>


RiscvCpu::RiscvCpu() {
  for (int i = 0; i < kRegNum; i++) {
    reg[i] = 0;
  }
  csrs.resize(kCsrSize);
}

bool RiscvCpu::check(bool x) {
  if (x) {
    std::cerr << "SRA sign error." << std::endl;
    return true;
  }
  return false;
}

void RiscvCpu::set_register(uint32_t num, uint32_t value) { reg[num] = value; }

void RiscvCpu::set_memory(std::shared_ptr<memory_wrapper> memory) {
  this->memory = memory;
}

uint32_t RiscvCpu::load_cmd(uint32_t pc) {
  auto &mem = *memory;
  return mem[pc] | (mem[pc + 1] << 8) | (mem[pc + 2] << 16) | (mem[pc + 3] << 24);
}

uint32_t RiscvCpu::read_register(uint32_t num) {
  return reg[num];
}

/* The definition of the Linux system call can be found in
 * riscv-gnu-toolchain/linux-headers/include/asm-generic/unistd.h
 */

std::pair<bool, bool> RiscvCpu::system_call() {
  bool end_flag = false;
  bool error_flag = false;
  if (reg[A7] == 93) {
    // Exit system call.
    end_flag = true;
  } else if (reg[A7] == 64) {
    // Write system call.
    // Ignore fd. Always output to stdout.
    // char *str = reinterpret_cast<char *>(mem) + reg[A1];
    auto &mem = *memory;
    for (int i = reg[A1]; i < reg[A1] + reg[A2]; i++) {
      putchar(mem[i]);
    }
    reg[A0] = reg[A2];
  }
  // TOOD: Implement close (57), fstat(80), lseek(62), brk(214)

  return std::make_pair(error_flag, end_flag);
}

uint32_t RiscvCpu::load_wd(uint32_t address) {
  auto &mem = *memory;
  return mem[address] | (mem[address + 1] << 8) | (mem[address + 2] << 16) | (mem[address + 3] << 24);
}

void RiscvCpu::store_wd(uint32_t address, uint32_t data, int width) {
  auto &mem = *memory;
  switch(width) {
    case 32:
      mem[address + 2] = (data >> 16) & 0xFF;
      mem[address + 3] = (data >> 24) & 0xFF;
    case 16:
      mem[address + 1] = (data >> 8) & 0xFF;
    case 8:
      mem[address] = data & 0xFF;
      break;
    default:
      throw std::invalid_argument("Store width is not 8, 16, or 32.");
  }
}

int RiscvCpu::run_cpu(uint32_t start_pc, bool verbose) {
  bool error_flag = false;
  bool end_flag = false;

  if (verbose) {
    std::cout << "   PC       CMD       X0       X1       X2       X3       X4       "
           "X5       X6       X7       X8       X9      X10      X11      "
           "X12      X13      X14      X15      X16      X17      X18      "
           "X19      X20      X21      X22      X23      X24      X25      "
           "X26      X27      X28      X29      X30      X31" << std::endl;
  }

  // mem = this->memory->data();
  pc = start_pc;
  do {
    uint32_t next_pc;
    uint32_t ir;

    ir = load_cmd(pc);
    if (verbose) {
      printf(" %4x  %08x %08x %08x %08x %08x %08x %08x %08x %08x "
             "%08x %08x %08x %08x %08x %08x %08x %08x "
             "%08x %08x %08x %08x %08x %08x %08x %08x "
             "%08x %08x %08x %08x %08x %08x %08x %08x",
             pc, ir, reg[0], reg[1], reg[2], reg[3], reg[4], reg[5],
             reg[6], reg[7], reg[8], reg[9], reg[10], reg[11],
             reg[12], reg[13], reg[14], reg[15], reg[16], reg[17],
             reg[18], reg[19], reg[20], reg[21], reg[22], reg[23],
             reg[24], reg[25], reg[26], reg[27], reg[28], reg[29],
             reg[30], reg[31]
      );
      std::cout << std::endl;
    }

    next_pc = pc + 4;

    // Decode. Mimick the HW behavior. (In HW, decode is in parallel.)
    uint8_t instruction = get_code(ir);
    uint8_t rd = get_rd(ir);
    uint8_t rs1 = get_rs1(ir);
    uint8_t rs2 = get_rs2(ir);
    int16_t imm12 = get_imm12(ir);
    int16_t csr = get_csr(ir);
    uint8_t shamt = get_shamt(ir);
    int16_t imm13 = get_imm13(ir);
    int32_t imm21 = get_imm21(ir);
    int16_t imm12_stype = get_stype_imm12(ir);
    int32_t imm20 = get_imm20(ir);
    uint32_t address;
    int32_t sreg_rs1, sreg_rs2;

    switch (instruction) {
      uint32_t t;
      case INST_ADD:
        reg[rd] = reg[rs1] + reg[rs2];
        break;
      case INST_AND:
        reg[rd] = reg[rs1] & reg[rs2];
        break;
      case INST_SUB:
        reg[rd] = reg[rs1] - reg[rs2];
        break;
      case INST_OR:
        reg[rd] = reg[rs1] | reg[rs2];
        break;
      case INST_XOR:
        reg[rd] = reg[rs1] ^ reg[rs2];
        break;
      case INST_SLL:
        reg[rd] = reg[rs1] << (reg[rs2] & 0x1F);
        break;
      case INST_SRL:
        reg[rd] = reg[rs1] >> (reg[rs2] & 0x1F);
        break;
      case INST_SRA:
        reg[rd] = static_cast<int32_t>(reg[rs1]) >> (reg[rs2] & 0x1F);
        check((reg[rs1] & 0x1F <= 0) || (reg[rs1] & 0x80000000 == 0));
        break;
      case INST_SLT:
        reg[rd] = (static_cast<int32_t>(reg[rs1]) < static_cast<int32_t>(reg[rs2])) ? 1 : 0;
        break;
      case INST_SLTU:
        reg[rd] = (reg[rs1] < reg[rs2]) ? 1 : 0;
        break;
      case INST_ADDI:
        reg[rd] = reg[rs1] + imm12;
        break;
      case INST_ANDI:
        reg[rd] = reg[rs1] & imm12;
        break;
      case INST_ORI:
        reg[rd] = reg[rs1] | imm12;
        break;
      case INST_XORI:
        reg[rd] = reg[rs1] ^ imm12;
        break;
      case INST_SLLI:
        reg[rd] = reg[rs1] << shamt;
        check(shamt >> 5 & 1 == 0);
        break;
      case INST_SRLI:
        reg[rd] = reg[rs1] >> shamt;
        check(shamt >> 5 & 1 == 0);
        break;
      case INST_SRAI:
        reg[rd] = static_cast<int32_t>(reg[rs1]) >> shamt;
        check(shamt >> 5 & 1 == 0);
        break;
      case INST_SLTI:
        reg[rd] = static_cast<int32_t>(reg[rs1]) < imm12 ? 1 : 0;
        break;
      case INST_SLTIU:
        reg[rd] = reg[rs1] < static_cast<uint32_t>(imm12) ? 1 : 0;
        break;
      case INST_BEQ:
        if (reg[rs1] == reg[rs2]) {
          next_pc = pc + sext(imm13, 13);
        }
        break;
      case INST_BGE:
        sreg_rs1 = static_cast<int32_t>(reg[rs1]);
        sreg_rs2 = static_cast<int32_t>(reg[rs2]);
        if (sreg_rs1 >= sreg_rs2) {
          next_pc = pc + imm13;
        }
        break;
      case INST_BGEU:
        if (reg[rs1] >= reg[rs2]) {
          next_pc = pc + imm13;
        }
        break;
      case INST_BLT:
        if (static_cast<int32_t>(reg[rs1]) < static_cast<int32_t>(reg[rs2])) {
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
        if (next_pc == pc) {
          error_flag = true;
        }
        break;
      case INST_JALR:
        next_pc = (reg[rs1] + imm12) & ~1;
        reg[rd] = pc + 4;
        if (rd == ZERO && rs1 == RA && reg[rs1]==0 && imm12 == 0) {
          end_flag = true;
        }
        break;
      case INST_LB:
        address = reg[rs1] + imm12;
        reg[rd] = sext(load_wd(address) & 0xFF, 8);
        break;
      case INST_LBU:
        address = reg[rs1] + imm12;
        reg[rd] = load_wd(address) & 0xFF;
        break;
      case INST_LH:
        address = reg[rs1] + imm12;
        reg[rd] = sext(load_wd(address) & 0xFFFF, 16);
        break;
      case INST_LHU:
        address = reg[rs1] + imm12;
        reg[rd] = load_wd(address) & 0xFFFF;
        break;
      case INST_LW:
        address = reg[rs1] + imm12;
        reg[rd] = load_wd(address);
        break;
      case INST_SW:
        address = reg[rs1] + imm12_stype;
        store_wd(address, reg[rs2]);
        break;
      case INST_SH:
        address = reg[rs1] + imm12_stype;
        store_wd(address, reg[rs2], 16);
        break;
      case INST_SB:
        address = reg[rs1] + imm12_stype;
        store_wd(mem + address, reg[rs2], 8);
        break;
      case INST_LUI:
        reg[rd] = imm20 << 12;
        break;
      case INST_AUIPC:
        reg[rd] = pc + (imm20 << 12);
        break;
      case INST_SYSTEM:
        if (imm12 == 0b000000000000) {
          // ECALL
          std::tie(error_flag, end_flag) = system_call();
        } else if (imm12 == 0b000000000001) {
          // EBREAK
          // Debug function is not implemented yet.
        } else if (imm12 == 0b001100000010) {
          // MRET
          next_pc = csrs[kMepc];
          // TODO: Implement privilege mode change.
        } else {
          // not defined.
          error_flag = true;
        }
        break;
      case INST_CSRRC:
        t = csrs[csr];
        csrs[csr] &= ~reg[rs1];
        reg[rd] = t;
        break;
      case INST_CSRRCI:
        t = csrs[csr];
        csrs[csr] &= ~rs1;
        reg[rd] = t;
        break;
      case INST_CSRRS:
        t = csrs[csr];
        csrs[csr] |= reg[rs1];
        reg[rd] = t;
        break;
      case INST_CSRRSI:
        t = csrs[csr];
        csrs[csr] |= rs1;
        reg[rd] = t;
        break;
      case INST_CSRRW:
        t = csrs[csr];
        csrs[csr] = reg[rs1];
        reg[rd] = t;
        break;
      case INST_CSRRWI:
        t = csrs[csr];
        csrs[csr] = rs1;
        reg[rd] = t;
        break;
      case INST_FENCE:
      case INST_FENCEI:
        // Do nothing for now.
        break;
      case INST_ERROR:
      default:
        error_flag = true;
        break;
    }
    reg[ZERO] = 0;
    if (pc == next_pc) {
      std::cerr << "Infinite loop detected." << std::endl;
      error_flag = true;
    }
    pc = next_pc & 0xFFFFFFFF;
  } while (!error_flag && !end_flag);

  return error_flag;
}

uint32_t RiscvCpu::get_code(uint32_t ir) {
  uint16_t opcode = bitcrop(ir, 7, 0);
  uint8_t funct3 = bitcrop(ir, 3, 12);
  uint8_t funct7 = bitcrop(ir, 7, 25);
  uint32_t instruction = INST_ERROR;
  switch (opcode) {
    case OPCODE_ARITHLOG: // ADD, SUB
      if (funct3 == FUNC3_ADDSUB) {
        instruction = (funct7 == FUNC_NORM) ? INST_ADD : INST_SUB;
      } else if (funct3 == FUNC3_AND) {
        instruction = INST_AND;
      } else if (funct3 == FUNC3_OR) {
        instruction = INST_OR;
      } else if (funct3 == FUNC3_XOR) {
        instruction = INST_XOR;
      } else if (funct3 == FUNC3_SR) {
        instruction = (funct7 == FUNC_NORM) ? INST_SRL : INST_SRA;
      } else if (funct3 == FUNC3_SL) {
        instruction = INST_SLL;
      } else if (funct3 == FUNC3_SLT) {
        instruction = INST_SLT;
      } else if (funct3 == FUNC3_SLTU) {
        instruction = INST_SLTU;
      }
      break;
    case OPCODE_ARITHLOG_I: // ADDI, SUBI
      if (funct3 == FUNC3_ADDSUB) {
        instruction = INST_ADDI;
      } else if (funct3 == FUNC3_AND) {
        instruction = INST_ANDI;
      } else if (funct3 == FUNC3_OR) {
        instruction = INST_ORI;
      } else if (funct3 == FUNC3_XOR) {
        instruction = INST_XORI;
      } else if (funct3 == FUNC3_SL) {
        instruction = INST_SLLI;
      } else if (funct3 == FUNC3_SR) {
        instruction = (funct7 == FUNC_NORM) ? INST_SRLI : INST_SRAI;
      } else if (funct3 == FUNC3_SLT) {
        instruction = INST_SLTI;
      } else if (funct3 == FUNC3_SLTU) {
        instruction = INST_SLTIU;
      }
      break;
    case OPCODE_B: // beq, bltu, bge, bne
      if (funct3 == FUNC3_BEQ) {
        instruction = INST_BEQ;
      } else if (funct3 == FUNC3_BLT) {
        instruction = INST_BLT;
      } else if (funct3 == FUNC3_BLTU) {
        instruction = INST_BLTU;
      } else if (funct3 == FUNC3_BGE) {
        instruction = INST_BGE;
      } else if (funct3 == FUNC3_BGEU) {
        instruction = INST_BGEU;
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
      if (funct3 == FUNC3_LSB) {
        instruction = INST_LB;
      } else if (funct3 == FUNC3_LSBU) {
        instruction = INST_LBU;
      } else if (funct3 == FUNC3_LSH) {
        instruction = INST_LH;
      } else if (funct3 == FUNC3_LSHU) {
        instruction = INST_LHU;
      } else if (funct3 == FUNC3_LSW) {
        instruction = INST_LW;
      }
      break;
    case OPCODE_S: // SW
      if (funct3 == FUNC3_LSW) {
        instruction = INST_SW;
      } else if (funct3 == FUNC3_LSH) {
        instruction = INST_SH;
      } else if (funct3 == FUNC3_LSB) {
        instruction = INST_SB;
      }
      break;
    case OPCODE_LUI: // LUI
      instruction = INST_LUI;
      break;
    case OPCODE_AUIPC: // AUIPC
      instruction = INST_AUIPC;
      break;
    case OPCODE_SYSTEM: // EBREAK
      if (funct3 == FUNC3_SYSTEM) {
        instruction = INST_SYSTEM;
      } else if (funct3 == FUNC3_CSRRC) {
        instruction = INST_CSRRC;
      } else if (funct3 == FUNC3_CSRRCI) {
        instruction = INST_CSRRCI;
      } else if (funct3 == FUNC3_CSRRS) {
        instruction = INST_CSRRS;
      } else if (funct3 == FUNC3_CSRRSI) {
        instruction = INST_CSRRSI;
      } else if (funct3 == FUNC3_CSRRW) {
        instruction = INST_CSRRW;
      } else if (funct3 == FUNC3_CSRRWI) {
        instruction = INST_CSRRWI;
      }
      break;
    case OPCODE_FENCE:
      if (funct3 == FUNC3_FENCEI) {
        instruction = INST_FENCEI;
      } else if (funct3 == FUNC3_FENCE) {
        instruction = INST_FENCE;
      }
     break;
    default:
      break;
  }
  if (instruction == INST_ERROR) {
    printf("Error decoding 0x%08x\n", ir);
  }
  return instruction;
}


