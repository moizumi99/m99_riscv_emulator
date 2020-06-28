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
// Disassemble16 and GetCode16 have very similar logic. Should be combined in some way.
std::string Disassemble16(uint32_t ir, int mxl = 1) {
  std::string cmd = "Unsupported C Instruction";
  uint32_t instruction, rd, rs1, rs2;
  int32_t imm;
  std::tie(instruction, rd, rs1, rs2, imm) = RiscvCpu::GetCode16(ir, mxl);
  uint16_t opcode = (bitcrop(ir, 3, 13) << 2) | bitcrop(ir, 2, 0);
  switch (opcode) {
    case 0b00000:
      cmd = "C.ADDI4SPN " + GetRegName(rd) + ", SP, " + std::to_string(imm);
      break;
    case 0b00001:
      cmd = "C.ADDI " + GetRegName(rd) + ", " + std::to_string(imm);
      break;
    case 0b00010:
      cmd = "C.SLLI " + GetRegName(rd) + ", " + std::to_string(imm);
      break;
    case 0b00101:
      if (mxl == 1) {
        cmd = "C.JAL " + std::to_string(SignExtend(imm, 12));
      } else {
        cmd = "C.ADDIW " + GetRegName(rd) + ", " + std::to_string(imm);
      }
      break;
    case 0b01000:
      cmd = "C.LW " + GetRegName(rd) + ", " + std::to_string(imm) + "(" +
            GetRegName(rs1) + ")";
      break;
    case 0b01001:
      cmd = "C.LI " + GetRegName(rd) + ", " + std::to_string(imm);
      break;
    case 0b01010:
      cmd = "C.LWSP " + GetRegName(rd) + ", " + std::to_string(imm) + "(SP)";
      break;
    case 0b01100:
      cmd = "C.LD " + GetRegName(rd) + ", " + std::to_string(imm) + "(" +
            GetRegName(rs1) + ")";
      break;
    case 0b01101:
      if (bitcrop(ir, 5, 7) == 0b00010) {
        // c.addi16sp.
        cmd = "C.ADDI16SP SP, SP, " + std::to_string(imm);
      } else {
        cmd = "C.LUI " + GetRegName(rd) + ", " + std::to_string(imm);
      }
      break;
    case 0b01110:
      cmd = "C.LDSP " + GetRegName(rd) + ", " + std::to_string(imm) + "(SP)";
      break;
    case 0b10010: // c.add
      if (bitcrop(ir, 1, 12) == 1) {
        if (bitcrop(ir, 5, 2) == 0 && bitcrop(ir, 5, 7) == 0) {
          cmd = "C.EBREAK";
        } else if (bitcrop(ir, 5, 2) == 0) {
          cmd = "C.JALR " + GetRegName(rs1);
        } else if (bitcrop(ir, 5, 7) != 0) {
          cmd = "C.ADD " + GetRegName(rd) + ", " + GetRegName(rs2);
        }
      } else if (bitcrop(ir, 5, 2) == 0) {
        cmd = "C.JR " + GetRegName(rs1);
      } else {
        cmd = "C.MV " + GetRegName(rd) + ", " + GetRegName(rs2);
      }
      break;
    case 0b10001:
      if (bitcrop(ir, 3, 10) == 0b011 && bitcrop(ir, 2, 5) == 0b11) {
        // c.and.
        cmd = "C.AND " + GetRegName(rd) + ", " + GetRegName(rs2);
      } else if (bitcrop(ir, 3, 10) == 0b011 && bitcrop(ir, 2, 5) == 0b01) {
        cmd = "C.XOR " + GetRegName(rd) + ", " + GetRegName(rs2);
      } else if (bitcrop(ir, 3, 10) == 0b011 && bitcrop(ir, 2, 5) == 0b00) {
        cmd = "C.SUB " + GetRegName(rd) + ", " + GetRegName(rs2);
      } else if (bitcrop(ir, 3, 10) == 0b011 && bitcrop(ir, 2, 5) == 0b10) {
        cmd = "C.OR " + GetRegName(rd) + ", " + GetRegName(rs2);
      } else if (bitcrop(ir, 3, 10) == 0b111 && bitcrop(ir, 2, 5) == 0b01) {
        cmd = "C.ADDW " + GetRegName(rd) + ", " + GetRegName(rs2);
      } else if (bitcrop(ir, 3, 10) == 0b111 && bitcrop(ir, 2, 5) == 0b00) {
        cmd = "C.SUBW " + GetRegName(rd) + ", " + GetRegName(rs2);
      } else if (bitcrop(ir, 2, 10) == 0b01) {
        cmd = "C.SRAI " + GetRegName(rd) + ", " + std::to_string(imm);
      } else if (bitcrop(ir, 2, 10) == 0b00) {
        cmd = "C.SRLI " + GetRegName(rd) + ", " + std::to_string(imm);
      } else if (bitcrop(ir, 2, 10) == 0b10) {
        cmd = "C.ANDI " + GetRegName(rd) + ", " +
              std::to_string(SignExtend(imm, 6));
      }
      break;
    case 0b10101:
      cmd = "C.J " + std::to_string(SignExtend(imm, 12));
      break;
    case 0b11000:
      cmd = "C.SW " + GetRegName(rs2) + ", " + std::to_string(imm) + "(" +
            GetRegName(rs1) + ")";
    case 0b11001:
      cmd =
        "C.BEQZ " + GetRegName(rs1) + ", " + std::to_string(SignExtend(imm, 9));
      break;
    case 0b11010:
      cmd = "C.SWSP " + GetRegName(rs2) + ", " + std::to_string(imm) + "(SP)";
      break;
    case 0b11100:
      cmd = "C.SD " + GetRegName(rs2) + ", " + std::to_string(imm) + "(" +
            GetRegName(rs1) + ")";
      break;
    case 0b11101:
      cmd =
        "C.BNEZ " + GetRegName(rs1) + ", " + std::to_string(SignExtend(imm, 9));
      break;
    case 0b11110:
      cmd = "C.SDSP " + GetRegName(rs2) + ", " + std::to_string(imm) + "(SP)";
  }
  return cmd;
}


std::string Disassemble(uint32_t ir, int mxl) {
  uint16_t opcode = bitcrop(ir, 7, 0);
  if ((opcode & 0b11) != 0b11) {
    return Disassemble16(ir, mxl);
  }
  uint8_t funct3 = bitcrop(ir, 3, 12);
  uint8_t funct7 = bitcrop(ir, 7, 25);
  uint8_t funct5 = funct7 >> 2;
  uint32_t rd = GetRd(ir);
  uint32_t rs1 = GetRs1(ir);
  uint32_t rs2 = GetRs2(ir);
  int16_t imm12 = GetImm12(ir);
  int16_t csr = GetCsr(ir);
  int16_t imm13 = GetImm13(ir);
  int32_t imm21 = GetImm21(ir);
  int16_t imm12_stype = GetStypeImm12(ir);
  int32_t imm20 = GetImm20(ir);
  std::string cmd = "UNDEF";
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
        } else if (funct3 == FUNC3_DIV) {
          cmd = "DIV";
        } else if (funct3 == FUNC3_DIVU) {
          cmd = "DIVU";
        } else if (funct3 == FUNC3_REM) {
          cmd = "REM";
        } else if (funct3 == FUNC3_REMU) {
          cmd = "REMU";
        }
        cmd += " " + GetRegName(rd) + ", " + GetRegName(rs1) + ", " +
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
            } else if (funct3 == FUNC3_DIVU) {
              cmd = "DIVUW";
            } else if (funct3 == FUNC3_DIV) {
              cmd = "DIVW";
            } else if (funct3 == FUNC3_REMU) {
              cmd = "REMUW";
            } else if (funct3 == FUNC3_REM) {
              cmd = "REMW";
            }
          }
      }
      cmd += " " + GetRegName(rd) + ", " + GetRegName(rs1) + ", " +
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
      cmd = "JAL " + GetRegName(rd) + ", " + std::to_string(imm21);
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
      cmd += " " +
             GetRegName(rs2) + ", " + std::to_string(imm12_stype) + "(" +
             GetRegName(rs1) +
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
    case OPCODE_AMO:
      if (funct5 == FUNC5_AMOADD) {
        cmd = funct3 == FUNC3_AMOD ? "AMOADD.D" : "AMOADD.W";
      } else if (funct5 == FUNC5_AMOAND) {
        cmd = funct3 == FUNC3_AMOD ? "AMOAND.D" : "AMOAND.W";
      } else if (funct5 == FUNC5_AMOMAX) {
        cmd = funct3 == FUNC3_AMOD ? "AMOMAX.D" : "AMOMAX.W";
      } else if (funct5 == FUNC5_AMOMAXU) {
        cmd = funct3 == FUNC3_AMOD ? "AMOMAXU.D" : "AMOMAXU.W";
      }
      cmd += " " + GetRegName(rd) + ", " + GetRegName(rs2) + ", (" +
             GetRegName(rs1) + ")";
      break;
    default:
      cmd = "Undefined instruction";
      break;
  }
  return cmd;
}
}

