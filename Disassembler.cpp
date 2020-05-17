//
// Created by moiz on 5/16/20.
//

#include "Disassembler.h"
#include "instruction_encdec.h"

namespace RISCV_EMULATOR {

std::string GetRegName(int reg) {
  std::array<std::string, 32> kRegNames = {
    "ZERO", "RA", "SP", "GP", "TP", "T0", "T1", "T2",
    "FP", "S1", "A0", "A1", "A2", "A3", "A4", "A5",
    "A6", "A7", "S2", "S3", "S4", "S5", "S6", "S7",
    "S8", "S9", "S10", "S11", "T3", "T4", "T5", "T6",
  };
  return kRegNames[reg];
}

// TODO: Add tests for disassembly;

std::string Disassemble(uint32_t ir) {
  uint16_t opcode = bitcrop(ir, 7, 0);
  uint8_t funct3 = bitcrop(ir, 3, 12);
  uint8_t funct7 = bitcrop(ir, 7, 25);
  uint32_t rd = GetRd(ir);
  uint32_t rs1 = GetRs1(ir);
  uint32_t rs2 = GetRs2(ir);
  int16_t imm12 = GetImm12(ir);
  int16_t csr = GetCsr(ir);
  int16_t imm13 = GetImm13(ir);
  int32_t imm21 = GetImm21(ir);
  int16_t imm12_stype = GetStypeImm12(ir);
  int32_t imm20 = GetImm20(ir);
  std::string cmd;
  switch (opcode) {
    case OPCODE_ARITHLOG: // ADD, SUB
      if (funct7 == FUNC_NORM || funct7 == FUNC_ALT) {
        if (funct3 == FUNC3_ADDSUB) {
          cmd = (funct7 == FUNC_NORM) ? "ADD" : "SUB";
        } else if (funct3 == FUNC3_AND) {
          cmd = "AND";
        } else if (funct3 == FUNC3_OR) {
          cmd = "OR";
        } else if (funct3 == FUNC3_XOR) {
          cmd = "XOR";
        } else if (funct3 == FUNC3_SR) {
          if (funct7 == FUNC_NORM) {
            cmd = "SRL";
          } else if (funct7 == FUNC_ALT) {
            cmd = "SRA";
          }
        } else if (funct3 == FUNC3_SL) {
          cmd = "SLL";
        } else if (funct3 == FUNC3_SLT) {
          cmd = "SLT";
        } else if (funct3 == FUNC3_SLTU) {
          cmd = "SLTU";
        }
      } else if (funct7 == FUNC_MULT) {
        if (funct3 == FUNC3_MUL) {
          cmd = "MUL";
        } else if (funct3 == FUNC3_MULH) {
          cmd = "MULH";
        } else if (funct3 == FUNC3_MULHSU) {
          cmd = "MULHSU";
        } else if (funct3 == FUNC3_MULHU) {
          cmd = "MULHU";
        }
      }
      cmd += " " + GetRegName(rd) + ", " + GetRegName(rs1), +", " +
                                                              GetRegName(rs2);
      break;
    case OPCODE_ARITHLOG_64:
      if (funct7 == FUNC_NORM || funct7 == FUNC_ALT) {
        if (funct3 == FUNC3_ADDSUB) {
          cmd = (funct7 == FUNC_NORM) ? "ADDW" : "SUBW";
        } else if (funct3 == FUNC3_SL) {
          cmd = "SLLW";
        } else if (funct3 == FUNC3_SR) {
          if (funct7 == FUNC_NORM) {
            cmd = "SRLW";
          } else if (funct7 == FUNC_ALT) {
            cmd = "SRAW";
          }
        }
      } else if (funct7 == FUNC_MULT) {
        if (funct3 == FUNC3_MUL) {
          cmd = "MULW";
        }
      }
      cmd += " " + GetRegName(rd) + ", " + GetRegName(rs1), +", " +
                                                            GetRegName(rs2);
      break;
    case OPCODE_ARITHLOG_I: // ADDI, SUBI
      if (funct3 == FUNC3_ADDSUB) {
        cmd = "ADDI";
      } else if (funct3 == FUNC3_AND) {
        cmd = "ANDI";
      } else if (funct3 == FUNC3_OR) {
        cmd = "ORI";
      } else if (funct3 == FUNC3_XOR) {
        cmd = "XORI";
      } else if (funct3 == FUNC3_SL) {
        cmd = "SLLI";
      } else if (funct3 == FUNC3_SR) {
        if ((funct7 >> 1) == 0b000000) {
          cmd = "SRLI";
        } else if ((funct7 >> 1) == 0b010000) {
          cmd = "SRAI";
        }
        // If top 6 bits do not match, it's an error.
      } else if (funct3 == FUNC3_SLT) {
        cmd = "SLTI";
      } else if (funct3 == FUNC3_SLTU) {
        cmd = "SLTIU";
      }
      cmd += " " + GetRegName(rd) + ", " + GetRegName(rs1) + ", " +
             std::to_string(imm12);
      break;
    case OPCODE_ARITHLOG_I64:
      if (funct3 == FUNC3_ADDSUB) {
        cmd = "ADDIW";
      } else if (funct3 == FUNC3_SL) {
        cmd = "SLLIW";
      } else if (funct3 == FUNC3_SR) {
        if ((funct7 >> 1) == 0b000000) {
          cmd = "SRLIW";
        } else if ((funct7 >> 1) == 0b010000) {
          cmd = "SRAIW";
        }
      }
      cmd += " " + GetRegName(rd) + ", " + GetRegName(rs1) + ", " +
             std::to_string(imm12);
      break;
    case OPCODE_B: // beq, bltu, bge, bne
      if (funct3 == FUNC3_BEQ) {
        cmd = "BEQ";
      } else if (funct3 == FUNC3_BLT) {
        cmd = "BLT";
      } else if (funct3 == FUNC3_BLTU) {
        cmd = "BLTU";
      } else if (funct3 == FUNC3_BGE) {
        cmd = "BGE";
      } else if (funct3 == FUNC3_BGEU) {
        cmd = "BGEU";
      } else if (funct3 == FUNC3_BNE) {
        cmd = "BNE";
      }
      cmd += " " + GetRegName(rs1) + ", " + GetRegName(rs2) + ", " +
             std::to_string(imm13);
      break;
    case OPCODE_J: // jal
      cmd = "JAL " + GetRegName(rd) + std::to_string(imm21);
      break;
    case OPCODE_JALR: // jalr
      if (funct3 == FUNC3_JALR) {
        cmd = "JALR " + GetRegName(rd) + ", " + std::to_string(imm12) + "(" +
              GetRegName(rs1) + ")";
      }
      break;
    case OPCODE_LD: // LW
      if (funct3 == FUNC3_LSB) {
        cmd = "LB";
      } else if (funct3 == FUNC3_LSBU) {
        cmd = "LBU";
      } else if (funct3 == FUNC3_LSH) {
        cmd = "LH";
      } else if (funct3 == FUNC3_LSHU) {
        cmd = "LHU";
      } else if (funct3 == FUNC3_LSW) {
        cmd = "LW";
      } else if (funct3 == FUNC3_LSWU) {
        cmd = "LWU";
      } else if (funct3 == FUNC3_LSD) {
        cmd = "LD";
      }
      cmd += " " + GetRegName(rd) + ", " + std::to_string(imm12) + "(" +
             GetRegName(rs1) + ")";
      break;
    case OPCODE_S: // SW
      if (funct3 == FUNC3_LSB) {
        cmd = "SB";
      } else if (funct3 == FUNC3_LSH) {
        cmd = "SH";
      } else if (funct3 == FUNC3_LSW) {
        cmd = "SW";
      } else if (funct3 == FUNC3_LSD) {
        cmd = "SD";
      }
      cmd +=
        GetRegName(rs2) + ", " + std::to_string(imm12_stype) + "(" + GetRegName(rs1) +
        ")";
      break;
    case OPCODE_LUI: // LUI
      cmd = "LUI";
      cmd += " " + GetRegName(rd) + ", " + std::to_string(imm20 << 12);
      break;
    case OPCODE_AUIPC: // AUIPC
      cmd = "AUIPC";
      cmd += " " + GetRegName(rd) + ", " + std::to_string(imm20 << 12);
      break;
    case OPCODE_SYSTEM: // EBREAK
      if (funct3 == FUNC3_SYSTEM) {
        if (imm12 == 0) {
          cmd = "ECALL";
        } else if (imm12 == 1) {
          cmd = "EBREAK";
        } else if (imm12 == 0b001100000010) {
          cmd = "MRET";
        } else if (imm12 == 0b000100000010) {
          cmd = "SRET";
        } else if (((imm12 >> 5) == 0b0001001) && (rd == 0b00000)) {
          cmd = "sfence.vma";
        } else {
          cmd = "Undefined System Instruction";
        }
      } else {
        if (funct3 == FUNC3_CSRRC) {
          cmd = "CSRRC";
        } else if (funct3 == FUNC3_CSRRCI) {
          cmd = "CSRRCI";
        } else if (funct3 == FUNC3_CSRRS) {
          cmd = "CSRRS";
        } else if (funct3 == FUNC3_CSRRSI) {
          cmd = "CSRRSI";
        } else if (funct3 == FUNC3_CSRRW) {
          cmd = "CSRRW";
        } else if (funct3 == FUNC3_CSRRWI) {
          cmd = "CSRRWI";
        }
        cmd += " " + GetRegName(rd) + ", " + std::to_string(csr) + ", " +
               GetRegName(rs1);
      }
      break;
    case OPCODE_FENCE:
      if (funct3 == FUNC3_FENCEI) {
        cmd = "FENCEI";
      } else if (funct3 == FUNC3_FENCE) {
        cmd = "FENCE";
      }
      break;
    default:
      cmd = "Undefined instruction";
      break;
  }
  return cmd;
}
}

