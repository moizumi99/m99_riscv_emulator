#include "RISCV_cpu.h"
#include "bit_tools.h"
#include "instruction_encdec.h"
#include <iostream>
#include <random>

using namespace std;

#define ASSERT(X) if (X) {printf("SRA sign error.\n"); error_flag = true;}

constexpr int kRegNum = 32;

RiscvCpu::RiscvCpu(bool randomize) {
    if (randomize) {
        randomize_registers();
    } else {
        for (int i = 0; i < kRegNum; i++) {
            reg[i] = 0;
        }
    }
}

void RiscvCpu::set_register(uint32_t num, uint32_t value) { reg[num] = value; }

uint32_t RiscvCpu::load_cmd(uint8_t *mem, uint32_t pc) {
    uint8_t *address = mem + pc;
    return *address | (*(address + 1) << 8) | (*(address + 2) << 16) |
           (*(address + 3) << 24);
}


uint32_t RiscvCpu::read_register(uint32_t num) {
    return reg[num];
}

void RiscvCpu::randomize_registers() {
    std::mt19937_64 gen(std::random_device{}());

    for (int i = 1; i < 32; i++) {
        reg[i] = gen() & 0xFFFFFFFF;
    }
    reg[0] = 0;
}


int RiscvCpu::run_cpu(uint8_t *mem, uint32_t start_pc, bool verbose) {
    bool error_flag = false;
    bool end_flag = false;

    if (verbose) {
        printf("   PC    Binary       X0       X1       X2      X3       X3       X4       "
               "X5       X6       X7       X8      X10      X11      X12      "
               "X13      X14      X15      X16      X17      X18      X19      "
               "X20      X21      X22      X23      X24      X25      X26      "
               "X27      X28      X29      X30      X31\n");
    }

    pc = start_pc;
    do {
        uint32_t next_pc;
        uint32_t ir;

        ir = load_cmd(mem, pc);
        if (verbose) {
            printf(" %4x  %08x %08x %08x %08x %08x %08x %08x %08x %08x "
                   "%08x %08x %08x %08x %08x %08x %08x %08x "
                   "%08x %08x %08x %08x %08x %08x %08x %08x "
                   "%08x %08x %08x %08x %08x %08x %08x %08x \n",
                   pc, ir, reg[0], reg[1], reg[2], reg[3], reg[4], reg[5],
                   reg[6], reg[7], reg[8], reg[9], reg[10], reg[11],
                   reg[12], reg[13], reg[14], reg[15], reg[16], reg[17],
                   reg[18], reg[19], reg[20], reg[21], reg[22], reg[23],
                   reg[24], reg[25], reg[26], reg[27], reg[28], reg[29],
                   reg[30], reg[31]
            );
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
        int32_t imm20 = get_imm20(ir);
        uint32_t address;
        int32_t sreg_rs1, sreg_rs2;

        switch (instruction) {
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
                ASSERT((reg[rs1] & 0x1F <= 0) || (reg[rs1] & 0x80000000 == 0));
                break;
            case INST_SLT:
                reg[rd] = (static_cast<int32_t>(reg[rs1] < static_cast<int32_t>(reg[rs2]))) ? 1 : 0;
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
                ASSERT(shamt >> 5 & 1 == 0);
                break;
            case INST_SRLI:
                reg[rd] = reg[rs1] >> shamt;
                ASSERT(shamt >> 5 & 1 == 0);
                break;
            case INST_SRAI:
                reg[rd] = static_cast<int32_t>(reg[rs1]) >> shamt;
                ASSERT(shamt >> 5 & 1 == 0);
                break;
            case INST_SLTI:
                reg[rd] = static_cast<int32_t>(reg[rs1]) < imm12 ? 1 : 0;
                break;
            case INST_SLTIU:
                reg[rd] = reg[rs1] < static_cast<uint32_t>(imm12) ? 1 : 0;
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
            case INST_LUI:
                reg[rd] = (reg[rd] & 0x00000FFF) | (imm20 << 12);
                break;
            case INST_ERROR:
            default:
                error_flag = true;
                break;
        }
        reg[ZERO] = 0;

        pc = next_pc & 0xFFFF;
    } while (!error_flag && !end_flag);

    return error_flag;
}

uint32_t RiscvCpu::get_code(uint32_t ir) {
    uint16_t opcode = bitcrop(ir, 7, 0);
    uint8_t funct3 = bitcrop(ir, 3, 12);
    uint8_t funct7 = bitcrop(ir, 7, 25);
    uint32_t instruction = 0;
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
            if (funct3 == FUNC3_LS) {
                instruction = INST_LW;
            }
            break;
        case OPCODE_S: // SW
            if (funct3 == FUNC3_LS) {
                instruction = INST_SW;
            }
            break;
        case OPCODE_LUI: // LUI
            instruction = INST_LUI;
            break;
        default:
            instruction = INST_ERROR;
            break;
    }
    if (instruction == INST_ERROR) {
        printf("Error decoding 0x%08x\n", ir);
    }
    return instruction;
}
