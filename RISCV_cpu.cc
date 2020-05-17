#include "RISCV_cpu.h"
#include "bit_tools.h"
#include "instruction_encdec.h"
#include "memory_wrapper.h"
#include "system_call_emulator.h"
#include "Mmu.h"
#include "Disassembler.h"
#include <iostream>
#include <tuple>
#include <stdint.h>
#include <cassert>

namespace RISCV_EMULATOR {

RiscvCpu::RiscvCpu(bool en64bit) {
  mxl_ = en64bit ? 2 : 1;
  xlen_ = en64bit ? 64 : 32;
  privilege_ = PrivilegeMode::MACHINE_MODE;
  mstatus_ = 0;
  for (int i = 0; i < kRegNum; i++) {
    reg_[i] = 0;
  }
  InitializeCsrs();

}

RiscvCpu::RiscvCpu() : RiscvCpu(false) {}

void RiscvCpu::InitializeCsrs() {
  csrs_.resize(kCsrSize, 0);
  // U: User mode, S: Supervisor mode, N: User-level Interrupts, I: RV32I/RV64I base ISA.
  const uint32_t extensions = 0b00000101000010000100000000;
  csrs_[MISA] = (mxl_ << (xlen_ - 2)) | extensions;
  if (mxl_ == 2) {
    // For 64 bit mode, sxl and uxl are the same as mxl.
    csrs_[MSTATUS] = (static_cast<uint64_t >(mxl_) << 34) |
                     (static_cast<uint64_t>(mxl_) << 32);
  }
}

uint64_t
RiscvCpu::VirtualToPhysical(uint64_t virtual_address, bool write_access) {
  Mmu mmu(memory_, mxl_);
  mmu.SetPrivilege(privilege_);
  uint64_t physical_address = mmu.VirtualToPhysical(virtual_address,
                                                    csrs_[SATP], write_access);
  if (mmu.GetPageFault()) {
    page_fault_ = true;
    faulting_address_ = mmu.GetFaultingAddress();
  }
  return physical_address;
}

// A helper function to record shift sign error.
bool RiscvCpu::CheckShiftSign(uint8_t shamt, uint8_t instruction,
                              const std::string &message_str) {
  if (xlen_ == 32 || instruction == INST_SLLIW || instruction == INST_SRAIW ||
      instruction == INST_SRLIW) {
    if (shamt >> 5) {
      std::cerr << message_str << " Shift value (shamt) error. shamt = "
                << static_cast<int>(shamt) << std::endl;
      return true;
    }
  }
  return false;
}

void RiscvCpu::SetMemory(std::shared_ptr<MemoryWrapper> memory) {
  this->memory_ = memory;
}

uint32_t RiscvCpu::LoadCmd(uint64_t pc) {
  auto &mem = *memory_;
  uint64_t physical_address = VirtualToPhysical(pc);
  return mem.Read32(physical_address);
}

void RiscvCpu::SetRegister(uint32_t num, uint64_t value) { reg_[num] = value; }

uint64_t RiscvCpu::ReadRegister(uint32_t num) {
  return reg_[num];
}

void RiscvCpu::SetCsr(uint32_t index, uint64_t value) {
  csrs_[index] = value;
}

uint64_t RiscvCpu::ReadCsr(uint32_t index) {
  return csrs_[index];
}

void RiscvCpu::SetWorkMemory(uint64_t top, uint64_t bottom) {
  this->top_ = top;
  this->bottom_ = bottom;
}

/* The definition of the Linux system call can be found in
 * riscv-gnu-toolchain/linux-headers/include/asm-generic/unistd.h
 */

std::pair<bool, bool> RiscvCpu::SystemCall() {
  return SystemCallEmulation(memory_, reg_, top_, &brk_);
}

uint64_t RiscvCpu::LoadWd(uint64_t physical_address, int width) {
  assert(width == 32 || width == 64);
  auto &mem = *memory_;
  uint64_t result = (width == 32) ? mem.Read32(physical_address) : mem.Read64(
    physical_address);
  return result;
}

void RiscvCpu::StoreWd(uint64_t physical_address, uint64_t data, int width) {
  auto &mem = *memory_;
  switch (width) {
    case 64:
      mem[physical_address + 7] = (data >> 56) & 0xFF;
      mem[physical_address + 6] = (data >> 48) & 0xFF;
      mem[physical_address + 5] = (data >> 40) & 0xFF;
      mem[physical_address + 4] = (data >> 32) & 0xFF;
    case 32:
      mem[physical_address + 3] = (data >> 24) & 0xFF;
      mem[physical_address + 2] = (data >> 16) & 0xFF;
    case 16:
      mem[physical_address + 1] = (data >> 8) & 0xFF;
    case 8:
      mem[physical_address] = data & 0xFF;
      break;
    default:
      throw std::invalid_argument("Store width is not 8, 16, 32, or 64.");
  }
}

void RiscvCpu::Trap(int cause, bool interrupt) {
  // Currently supported exceptions: page fault (12, 13, 15) and ecall (8, 9, 11).
  assert(
    cause == INSTRUCTION_PAGE_FAULT || cause == LOAD_PAGE_FAULT ||
    cause == STORE_PAGE_FAULT || cause == ECALL_UMODE ||
    cause == ECALL_SMODE || cause == ECALL_MMODE);
  // Page fault specific processing.
  if (cause == INSTRUCTION_PAGE_FAULT || cause == LOAD_PAGE_FAULT ||
      cause == STORE_PAGE_FAULT) {
    if (prev_page_fault_ && prev_faulting_address_ == faulting_address_) {
      std::cerr
        << "This is a page fault right after another fault at the same address. Exit."
        << std::endl;
      error_flag_ = true;
    }
    prev_page_fault_ = page_fault_;
    prev_faulting_address_ = faulting_address_;
    page_fault_ = false;
  }

  // MTVAL, STVAL, and UTVAL.
  uint64_t tval = 0;
  if (cause == INSTRUCTION_PAGE_FAULT || cause == LOAD_PAGE_FAULT ||
      cause == STORE_PAGE_FAULT) {
    tval = faulting_address_;
  } else if (cause == ILLEGAL_INSTRUCTION) {
    tval = (*memory_)[pc_] | ((*memory_)[pc_ + 1] << 8) |
           ((*memory_)[pc_ + 2] << 16) | ((*memory_)[pc_ + 3] << 24);
  }
  // TODO: Implement other tval cases.

  // Check delegation status.
  bool machine_exception_delegation = ((csrs_[MEDELEG] >> cause) & 1) == 1;
  bool machine_interrupt_delegation = ((csrs_[MIDELEG] >> cause) & 1) == 1;
  bool sv_exception_delegation = ((csrs_[SEDELEG] >> cause) & 1) == 1;
  bool sv_interrupt_delegation = ((csrs_[SIDELEG] >> cause) & 1) == 1;

  bool sv_delegation = (interrupt && machine_interrupt_delegation) ||
                       (!interrupt && machine_exception_delegation);
  bool user_delegation = (interrupt && sv_interrupt_delegation) ||
                         (!interrupt && sv_exception_delegation);

  sv_delegation = sv_delegation &&
                  (privilege_ == PrivilegeMode::SUPERVISOR_MODE ||
                   privilege_ == PrivilegeMode::USER_MODE);
  user_delegation =
    user_delegation && (privilege_ == PrivilegeMode::USER_MODE) &&
    sv_delegation;

  if (user_delegation) {
    // Delegate to U-Mode.
    csrs_[UCAUSE] = cause;
    csrs_[UEPC] = pc_;
    uint64_t tvec = csrs_[UTVAL];
    uint8_t mode = tvec & 0b11;
    if (mode == 0 || interrupt) {
      next_pc_ = tvec & ~0b11;
    } else {
      next_pc_ =
        (tvec & ~0b11) + 4 * (csrs_[UCAUSE] & GenMask<uint64_t>(xlen_ - 1, 0));

    }
    csrs_[UTVAL] = tval;
    // Copy old UIE (#0) to UPIE ($4). Then, disable UIE (#0)
    const uint64_t uie = bitcrop(mstatus_, 1, 0);
    mstatus_ = bitset(mstatus_, 1, 4, uie);
    uint64_t new_uie = 0;
    mstatus_ = bitset(mstatus_, 1, 0, new_uie);
    privilege_ = PrivilegeMode::USER_MODE;
  } else if (sv_delegation) {
    // Delegated to S-Mode.
    csrs_[SCAUSE] = cause;
    csrs_[SEPC] = pc_;
    uint64_t tvec = csrs_[STVEC];
    uint8_t mode = tvec & 0b11;
    if (mode == 0 || interrupt) {
      next_pc_ = tvec & ~0b11;
    } else {
      next_pc_ =
        (tvec & ~0b11) + 4 * (csrs_[SCAUSE] & GenMask<uint64_t>(xlen_ - 1, 0));
    }
    csrs_[STVAL] = tval;

    // Copy old SIE (#1) to SPIE (#5). Then, disable SIE (#1).
    const uint64_t sie = bitcrop(mstatus_, 1, 1);
    mstatus_ = bitset(mstatus_, 1, 5, sie);
    uint64_t new_sie = 0;
    mstatus_ = bitset(csrs_[SSTATUS], 1, 1, new_sie);

    // Save the current privilege mode to SPP (#12-11), and set the privilege to Supervisor.
    uint64_t new_spp = static_cast<int>(privilege_) & 1;
    mstatus_ = bitset(mstatus_, 1, 8, new_spp);
    privilege_ = PrivilegeMode::SUPERVISOR_MODE;
  } else {
    csrs_[MCAUSE] = cause;
    csrs_[MEPC] = pc_;
    uint64_t tvec = csrs_[MTVEC];
    uint8_t mode = tvec & 0b11;
    if (mode == 0 || interrupt) {
      next_pc_ = tvec & ~0b11;
    } else {
      next_pc_ =
        (tvec & ~0b11) + 4 * (csrs_[MCAUSE] & GenMask<uint64_t>(xlen_ - 1, 0));
    }
    // Store the faulting address to MTVAL.
    csrs_[MTVAL] = tval;

    // Copy old MIE (#3) to MPIE (#7). Then, disable MIE.
    const uint64_t mie = bitcrop(mstatus_, 1, 3);
    mstatus_ = bitset(mstatus_, 1, 7, mie);
    uint64_t new_mie = 0;
    mstatus_ = bitset(mstatus_, 1, 3, new_mie);

    // Save the current privilege mode to MPP (#12-11), and set the privilege to Machine.
    uint64_t new_mpp = static_cast<int>(privilege_);
    mstatus_ = bitset(mstatus_, 2, 11, new_mpp);
    privilege_ = PrivilegeMode::MACHINE_MODE;
  }
  ApplyMstatusToCsr();
}

uint64_t kUpper32bitMask = 0xFFFFFFFF00000000;

uint64_t RiscvCpu::Sext32bit(uint64_t data32bit) {
  if ((data32bit >> 31) & 1) {
    return data32bit | kUpper32bitMask;
  } else {
    return data32bit & (~kUpper32bitMask);
  }
}

void RiscvCpu::CsrsInstruction(uint32_t instruction, uint32_t csr, uint32_t rd,
                               uint32_t rs1) {
  uint64_t t = csrs_[csr];
  switch (instruction) {
    case INST_CSRRC:
      csrs_[csr] &= ~reg_[rs1];
      break;
    case INST_CSRRCI:
      csrs_[csr] &= ~rs1;
      break;
    case INST_CSRRS:
      csrs_[csr] |= reg_[rs1];
      break;
    case INST_CSRRSI:
      csrs_[csr] |= rs1;
      break;
    case INST_CSRRW:
      csrs_[csr] = reg_[rs1];
      break;
    case INST_CSRRWI:
      csrs_[csr] = rs1;
      break;
    default:;
  }
  reg_[rd] = t;
  UpdateMstatus(csr);
}

uint64_t
RiscvCpu::BranchInstruction(uint32_t instruction, uint32_t rs1, uint32_t rs2,
                            int32_t imm13) {
  bool condition = false;
  switch (instruction) {
    case INST_BEQ:
      condition = reg_[rs1] == reg_[rs2];
      break;
    case INST_BGE:
      condition = static_cast<int64_t>(reg_[rs1]) >=
                  static_cast<int64_t>(reg_[rs2]);
      break;
    case INST_BGEU:
      condition = reg_[rs1] >= reg_[rs2];
      break;
    case INST_BLT:
      condition =
        static_cast<int64_t>(reg_[rs1]) < static_cast<int64_t>(reg_[rs2]);
      break;
    case INST_BLTU:
      condition = reg_[rs1] < reg_[rs2];
      break;
    case INST_BNE:
      condition = reg_[rs1] != reg_[rs2];
      break;
    default:;
  }
  return condition ? pc_ + imm13 : next_pc_;
}

void
RiscvCpu::OperationInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1,
                               uint32_t rs2) {
  bool w_instruction = instruction == INST_ADDW || instruction == INST_SUBW ||
                       instruction == INST_SLLW || instruction == INST_SRLW ||
                       instruction == INST_SRAW;
  uint32_t shift_mask = (xlen_ == 32 || w_instruction) ? 0b0011111 : 0b0111111;
  uint64_t temp64;
  switch (instruction) {
    case INST_ADD:
    case INST_ADDW:
      temp64 = reg_[rs1] + reg_[rs2];
      break;
    case INST_SUB:
    case INST_SUBW:
      temp64 = reg_[rs1] - reg_[rs2];
      break;
    case INST_AND:
      temp64 = reg_[rs1] & reg_[rs2];
      break;
    case INST_OR:
      temp64 = reg_[rs1] | reg_[rs2];
      break;
    case INST_XOR:
      temp64 = reg_[rs1] ^ reg_[rs2];
      break;
    case INST_SLL:
    case INST_SLLW:
      temp64 = reg_[rs1] << (reg_[rs2] & shift_mask);
      break;
    case INST_SRL:
    case INST_SRLW:
      temp64 = reg_[rs1];
      if (xlen_ == 32 || instruction == INST_SRLW) {
        temp64 &= 0xFFFFFFFF;
      }
      temp64 = temp64 >> (reg_[rs2] & shift_mask);
      break;
    case INST_SRA:
      temp64 = static_cast<int64_t>(reg_[rs1]) >> (reg_[rs2] & shift_mask);
      break;
    case INST_SRAW:
      temp64 = Sext32bit(reg_[rs1]) >> (reg_[rs2] & shift_mask);
      break;
  }
  if (xlen_ == 32 || w_instruction) {
    temp64 = Sext32bit(temp64);
  }
  reg_[rd] = temp64;
}

void
RiscvCpu::ImmediateInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1,
                               int32_t imm12) {
  uint64_t temp64;
  switch (instruction) {
    case INST_ADDI:
    case INST_ADDIW:
      temp64 = reg_[rs1] + imm12;
      break;
    case INST_ANDI:
      temp64 = reg_[rs1] & imm12;
      break;
    case INST_ORI:
      temp64 = reg_[rs1] | imm12;
      break;
    case INST_XORI:
      temp64 = reg_[rs1] ^ imm12;
      break;
  }
  if (xlen_ == 32 || instruction == INST_ADDIW) {
    temp64 = Sext32bit(temp64);
  }
  reg_[rd] = temp64;
}

void RiscvCpu::ImmediateShiftInstruction(uint32_t instruction, uint32_t rd,
                                         uint32_t rs1, uint32_t shamt) {
  uint64_t temp64;
  switch (instruction) {
    case INST_SLLI:
      temp64 = reg_[rs1] << shamt;
      if (xlen_ == 32) {
        temp64 = Sext32bit(temp64);
      }
      reg_[rd] = temp64;
      CheckShiftSign(shamt, instruction, "SLLI");
      break;
    case INST_SLLIW:
      temp64 = (reg_[rs1] << shamt) & 0xFFFFFFFF;
      temp64 = Sext32bit(temp64);
      reg_[rd] = temp64;
      CheckShiftSign(shamt, instruction, "SLLIW");
      break;
    case INST_SRLI:
      temp64 = reg_[rs1];
      if (xlen_ == 32) {
        temp64 &= ~kUpper32bitMask;
      }
      temp64 = temp64 >> shamt;
      if (xlen_ == 32) {
        temp64 = Sext32bit(temp64);
      }
      reg_[rd] = temp64;
      CheckShiftSign(shamt, instruction, "SRLI");
      break;
    case INST_SRLIW:
      temp64 = (reg_[rs1] & 0xFFFFFFFF) >> shamt;
      temp64 = Sext32bit(temp64);
      reg_[rd] = temp64;
      CheckShiftSign(shamt, instruction, "SRLIW");
      break;
    case INST_SRAI:
      reg_[rd] = static_cast<int64_t>(reg_[rs1]) >> shamt;
      CheckShiftSign(shamt, instruction, "SRAI");
      break;
    case INST_SRAIW:
      reg_[rd] = static_cast<int32_t>(reg_[rs1]) >> shamt;
      CheckShiftSign(shamt, instruction, "SRAI");
      break;
  }
}

void RiscvCpu::LoadInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1,
                               int32_t imm12) {
  uint64_t source_address = reg_[rs1] + imm12;
  uint64_t address = VirtualToPhysical(source_address);
  if (page_fault_) {
    Trap(ExceptionCode::LOAD_PAGE_FAULT);
    return;
  }
  uint64_t load_data;
  if (instruction == INST_LB) {
    load_data = SignExtend(LoadWd(address) & 0xFF, 8); // LB
  } else if (instruction == INST_LBU) {
    load_data = LoadWd(address) & 0xFF; // LBU
  } else if (instruction == INST_LH) {
    load_data = SignExtend(LoadWd(address) & 0xFFFF, 16); // LH
  } else if (instruction == INST_LHU) {
    load_data = LoadWd(address) & 0xFFFF; // LHU
  } else if (instruction == INST_LW) {
    load_data = SignExtend(LoadWd(address), 32); // LW
  } else if (instruction == INST_LWU) {
    load_data = LoadWd(address); // LWU
  } else { // instruction == INST_LD
    load_data = LoadWd(address, 64); // LD
  }
  reg_[rd] = load_data;
}

void RiscvCpu::StoreInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1,
                                uint32_t rs2, int32_t imm12_stype) {
  uint64_t dst_address = reg_[rs1] + imm12_stype;
  uint64_t address = VirtualToPhysical(dst_address, true);
  if (page_fault_) {
    Trap(ExceptionCode::STORE_PAGE_FAULT);
    return;
  }
  int width;
  if (instruction == INST_SB) {
    width = 8;
  } else if (instruction == INST_SH) {
    width = 16;
  } else if (instruction == INST_SW) {
    width = 32;
  } else { // if instruction === INST_SD.
    width = 64;
  }
  StoreWd(address, reg_[rs2], width);
  if (mxl_ == 1) {
    host_write_ |= (address & 0xFFFFFFFF) == kToHost;
  } else {
    host_write_ |= address == kToHost;
  }
}

void
RiscvCpu::SystemInstruction(uint32_t instruction, uint32_t rd, int32_t imm12) {
  if (imm12 == 0b000000000000) {
    // ECALL
    if (ecall_emulation_) {
      std::tie(error_flag_, end_flag_) = SystemCall();
    } else {
      Ecall();
    }
  } else if (imm12 == 0b000000000001) {
    // EBREAK
    // Debug function is not implemented yet.
  } else if (imm12 == 0b001100000010) {
    Mret();
  } else if (imm12 == 0b000100000010) {
    Sret();
  } else if (((imm12 >> 5) == 0b0001001) && (rd == 0b00000)) {
    // sfence.vma.
    // TODO: Implement this function.
    ;
  } else {
    // not defined.
    std::cerr << "Undefined System instruction." << std::endl;
    error_flag_ = true;
  }
}

uint64_t GetMulh64(int64_t op1, int64_t op2) {
  int64_t a = op1 >> 32;
  uint64_t b = op1 & 0xFFFFFFFF;
  int64_t c = op2 >> 32;
  uint64_t d = op2 & 0xFFFFFFFF;
  int64_t ac = a * c;
  int64_t ad = a * d;
  int64_t bc = b * c;
  uint64_t bd = b * d;

  // Break down into 32 bit segments;
  uint64_t d32 = (ad & 0xFFFFFFFF) + (bc & 0xFFFFFFFF) + (bd >> 32);

  int64_t upper = ac + (ad >> 32) + (bc >> 32) + (d32 >> 32);
  return static_cast<uint64_t>(upper);
}

uint64_t GetMulhsu64(int64_t op1, uint64_t op2) {
  int64_t a = op1 >> 32;
  uint64_t b = op1 & 0xFFFFFFFF;
  uint64_t c = op2 >> 32;
  uint64_t d = op2 & 0xFFFFFFFF;
  int64_t ac = a * c;
  int64_t ad = a * d;
  uint64_t bc = b * c;
  uint64_t bd = b * d;

  // Break down into 32 bit segments;
  uint64_t d32 = (ad & 0xFFFFFFFF) + (bc & 0xFFFFFFFF) + (bd >> 32);

  int64_t upper = ac + (ad >> 32) + (bc >> 32) + (d32 >> 32);
  return static_cast<uint64_t>(upper);
}

uint64_t GetMulhu64(uint64_t op1, uint64_t op2) {
  uint64_t a = op1 >> 32;
  uint64_t b = op1 & 0xFFFFFFFF;
  uint64_t c = op2 >> 32;
  uint64_t d = op2 & 0xFFFFFFFF;
  uint64_t ac = a * c;
  uint64_t ad = a * d;
  uint64_t bc = b * c;
  uint64_t bd = b * d;

  // Break down into 32 bit segments;
  uint64_t d32 = (ad & 0xFFFFFFFF) + (bc & 0xFFFFFFFF) + (bd >> 32);

  uint64_t upper = ac + (ad >> 32) + (bc >> 32) + (d32 >> 32);
  return upper;
}

void
RiscvCpu::MultInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1,
                          uint32_t rs2) {
  bool sign_extend_en = xlen_ == 32;
  uint64_t temp64;
  uint64_t largest_negative = static_cast<uint64_t>(1) << (xlen_ - 1);
  switch (instruction) {
    case INST_MUL:
      temp64 = reg_[rs1] * reg_[rs2];
      break;
    case INST_MULW:
      temp64 = reg_[rs1] * reg_[rs2];
      sign_extend_en = true;
      break;
    case INST_MULH:
      if (xlen_ == 32) {
        temp64 =
          (static_cast<int64_t>(reg_[rs1]) * static_cast<int64_t>(reg_[rs2]))
            >> 32;
      } else {
        temp64 = GetMulh64(reg_[rs1],
                           reg_[rs2]);
      }
      break;
    case INST_MULHSU:
      if (xlen_ == 32) {
        temp64 = (static_cast<int64_t>(reg_[rs1]) *
          (reg_[rs2] & 0xFFFFFFFF)) >> 32;
      } else {
        temp64 = GetMulhsu64(static_cast<int64_t>(reg_[rs1]),
                             reg_[rs2]);
      }
      break;
    case INST_MULHU:
      if (xlen_ == 32) {
        temp64 = ((reg_[rs1] & 0xFFFFFFFF) *
                  (reg_[rs2] & 0xFFFFFFFF)) >> 32;
      } else {
        temp64 = GetMulhu64(reg_[rs1],
                            reg_[rs2]);
      }
      break;
    case INST_DIV:
      if (reg_[rs2] == 0) {
        temp64 = -1;
      } else if (static_cast<int64_t>(reg_[rs2]) == -1 &&
                 reg_[rs1] == largest_negative) {
        // Overflow check.
        temp64 = largest_negative;
      } else {
        temp64 =
          static_cast<int64_t >(reg_[rs1]) / static_cast<int64_t>(reg_[rs2]);
      }
      break;
    case INST_DIVU:
      if (reg_[rs2] == 0) {
        temp64 = ~0;
      } else if (xlen_ == 32) {
        temp64 = static_cast<uint64_t>(reg_[rs1] & 0xFFFFFFFF) /
                 static_cast<uint64_t>(reg_[rs2] & 0xFFFFFFFF);
      } else {
        temp64 =
          static_cast<uint64_t >(reg_[rs1]) / static_cast<uint64_t>(reg_[rs2]);
      }
      break;
    case INST_DIVUW:
      if (reg_[rs2] == 0) {
        temp64 = ~0;
      } else {
        temp64 = (reg_[rs1] & 0xFFFFFFFF) / (reg_[rs2] & 0xFFFFFFFF);
      }
      sign_extend_en = true;
      break;
    case INST_DIVW:
      if (reg_[rs2] == 0) {
        temp64 = -1;
      } else if (static_cast<int64_t>(reg_[rs2]) == -1 &&
                 reg_[rs1] == 1 << 31) {
        // Overflow check.
        temp64 = 1 << 31;
      } else {
        temp64 = static_cast<int64_t>(SignExtend(reg_[rs1], 32)) /
                 static_cast<int64_t>(SignExtend(reg_[rs2], 32));
      }
      sign_extend_en = true;
      break;
    case INST_REM:
      if (reg_[rs2] == 0) {
        temp64 = reg_[rs1];
      } else if (static_cast<int64_t>(reg_[rs2]) == -1 &&
                 reg_[rs1] == largest_negative) {
        // Overflow check.
        temp64 = 0;
      } else {
        temp64 =
          static_cast<int64_t >(reg_[rs1]) % static_cast<int64_t>(reg_[rs2]);
      }
      break;
    case INST_REMU:
      if (reg_[rs2] == 0) {
        temp64 = reg_[rs1];
      } else if (xlen_ == 32) {
        temp64 = (reg_[rs1] & 0xFFFFFFFF) % (reg_[rs2] & 0xFFFFFFFF);
      } else {
        temp64 = reg_[rs1] % reg_[rs2];
      }
      break;
    case INST_REMUW:
      if (reg_[rs2] == 0) {
        temp64 = reg_[rs1];
      } else {
        temp64 = (reg_[rs1] & 0xFFFFFFFF) % (reg_[rs2] & 0xFFFFFFFF);
      }
      sign_extend_en = true;
      break;
    case INST_REMW:
      if (reg_[rs2] == 0) {
        temp64 = reg_[rs1];
      } else if (static_cast<int64_t>(reg_[rs2]) == -1 &&
                 reg_[rs1] == 1 << 31) {
        // Overflow check.
        temp64 = 0;
      } else {
        temp64 = static_cast<int64_t>(SignExtend(reg_[rs1], 32)) %
                 static_cast<int64_t>(SignExtend(reg_[rs2], 32));
      }
      sign_extend_en = true;
      break;
  }
  if (sign_extend_en) {
    temp64 = Sext32bit(temp64);
  }
  reg_[rd] = temp64;
}

int RiscvCpu::RunCpu(uint64_t start_pc, bool verbose) {
  error_flag_ = false;
  end_flag_ = false;
  host_write_ = false;

  next_pc_ = start_pc;
  do {
    pc_ = next_pc_;
    uint64_t ir;

    ir = LoadCmd(pc_);
    if (verbose) {
      char machine_status = privilege_ == PrivilegeMode::USER_MODE ? 'U' :
                            privilege_ == PrivilegeMode::SUPERVISOR_MODE ? 'S'
                                                                         : 'M';
      printf("%c %016lx (%08lx): ", machine_status, pc_, ir);
      std::cout << Disassemble(ir) << std::endl;
    }
    if (page_fault_) {
      Trap(ExceptionCode::INSTRUCTION_PAGE_FAULT);
      continue;
    }
    // Change this line when C is supported.
    next_pc_ = pc_ + 4;

    // Decode. Mimick the HW behavior. (In HW, decode is in parallel.)
    uint32_t instruction = GetCode(ir);
    uint32_t rd = GetRd(ir);
    uint32_t rs1 = GetRs1(ir);
    uint32_t rs2 = GetRs2(ir);
    int16_t imm12 = GetImm12(ir);
    int16_t csr = GetCsr(ir);
    uint8_t shamt = GetShamt(ir);
    int16_t imm13 = GetImm13(ir);
    int32_t imm21 = GetImm21(ir);
    int16_t imm12_stype = GetStypeImm12(ir);
    int32_t imm20 = GetImm20(ir);

    switch (instruction) {
      uint64_t temp64;
      case INST_ADD:
      case INST_ADDW:
      case INST_AND:
      case INST_SUB:
      case INST_SUBW:
      case INST_OR:
      case INST_XOR:
      case INST_SLL:
      case INST_SLLW:
      case INST_SRL:
      case INST_SRLW:
      case INST_SRA:
      case INST_SRAW:
        OperationInstruction(instruction, rd, rs1, rs2);
        break;
      case INST_ADDI:
      case INST_ADDIW:
      case INST_ANDI:
      case INST_ORI:
      case INST_XORI:
        ImmediateInstruction(instruction, rd, rs1, imm12);
        break;
      case INST_SLLI:
      case INST_SLLIW:
      case INST_SRLI:
      case INST_SRLIW:
      case INST_SRAI:
      case INST_SRAIW:
        ImmediateShiftInstruction(instruction, rd, rs1, shamt);
        break;
      case INST_SLT:
        reg_[rd] = (static_cast<int64_t>(reg_[rs1]) <
                    static_cast<int64_t>(reg_[rs2])) ? 1 : 0;
        break;
      case INST_SLTU:
        reg_[rd] = (reg_[rs1] < reg_[rs2]) ? 1 : 0;
        break;
      case INST_SLTI:
        reg_[rd] = static_cast<int64_t>(reg_[rs1]) < imm12 ? 1 : 0;
        break;
      case INST_SLTIU:
        reg_[rd] = reg_[rs1] < static_cast<uint64_t>(imm12) ? 1 : 0;
        break;
      case INST_BEQ:
      case INST_BGE:
      case INST_BGEU:
      case INST_BLT:
      case INST_BLTU:
      case INST_BNE:
        next_pc_ = BranchInstruction(instruction, rs1, rs2, imm13);
        break;
      case INST_JAL:
        reg_[rd] = pc_ + 4;
        next_pc_ = pc_ + imm21;
        if (next_pc_ == pc_) {
          error_flag_ = true;
        }
        break;
      case INST_JALR:
        next_pc_ = (reg_[rs1] + imm12) & ~1;
        reg_[rd] = pc_ + 4;
        // Below lines are only for simulation purpose.
        // Remove once a better solution is found.
        if (rd == ZERO && rs1 == RA && reg_[rs1] == 0 && imm12 == 0) {
          end_flag_ = true;
        }
        break;
      case INST_LB:
      case INST_LBU:
      case INST_LH:
      case INST_LHU:
      case INST_LW:
      case INST_LWU:
      case INST_LD:
        LoadInstruction(instruction, rd, rs1, imm12);
        break;
      case INST_SB:
      case INST_SH:
      case INST_SW:
      case INST_SD:
        StoreInstruction(instruction, rd, rs1, rs2, imm12_stype);
        break;
      case INST_LUI:
        temp64 = imm20 << 12;
        if (xlen_ == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_AUIPC:
        temp64 = pc_ + (imm20 << 12);
        if (xlen_ == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_SYSTEM:
        SystemInstruction(instruction, rd, imm12);
        break;
      case INST_CSRRC:
      case INST_CSRRCI:
      case INST_CSRRS:
      case INST_CSRRSI:
      case INST_CSRRW:
      case INST_CSRRWI:
        CsrsInstruction(instruction, csr, rd, rs1);
        break;
      case INST_FENCE:
      case INST_FENCEI:
        // Do nothing for now.
        break;
      case INST_MUL:
      case INST_MULH:
      case INST_MULHSU:
      case INST_MULHU:
      case INST_MULW:
      case INST_DIV:
      case INST_DIVU:
      case INST_DIVUW:
      case INST_DIVW:
      case INST_REM:
      case INST_REMU:
      case INST_REMUW:
      case INST_REMW:
        // RV32M/RV64M Instructions
        MultInstruction(instruction, rd, rs1, rs2);
        break;
      case INST_ERROR:
      default:
        std::cout << "Instruction Error at " << std::hex << pc_ << std::endl;
        error_flag_ = true;
        break;
    }
    if (pc_ == next_pc_) {
      std::cerr << "Infinite loop detected." << std::endl;
      error_flag_ = true;
    }
    reg_[ZERO] = 0;

    if (verbose) {
      DumpRegisters();
    }

    if (host_emulation_ && host_write_) {
      HostEmulation();
    }

  } while (!error_flag_ && !end_flag_);

  if (error_flag_ && verbose) {
    DumpCpuStatus();
  }
  return error_flag_;
}

// reference: https://github.com/riscv/riscv-isa-sim/issues/364
void RiscvCpu::HostEmulation() {
  uint64_t payload;
  uint8_t device;
  uint32_t command;
  uint64_t value = 0;
  if (mxl_ == 1) {
    // This address should be physical.
    payload = memory_->Read32(kToHost);
    device = 0;
    command = 0;
  } else {
    payload = memory_->Read64(kToHost);
    device = (payload >> 56) & 0xFF;
    command = (payload >> 48) & 0x3FFFF;
  }
  if (device == 0) {
    if (command == 0) {
      value = payload & 0xFFFFFFFFFFFF;
      if ((value & 1) == 0) {
        // Syscall emulation
        std::cerr << "Syscall Emulation Not Implemented Yet." << std::endl;
      } else {
        value = value >> 1;
        reg_[A0] = value;
        end_flag_ = true;
      }
    } else {
      std::cerr << "Unsupported Host command " << command << " for Device 0"
                << std::endl;
      error_flag_ = true;
      end_flag_ = true;
    }
  } else if (device == 1) {
    if (command == 1) {
      char character = value & 0xFF;
      std::cout << character;
    } else if (command == 0) {
      // TODO: Implement Read.
    } else {
      std::cerr << "Unsupported host command " << command << " for Device 1"
                << std::endl;
    }
  } else {
    std::cerr << "Unsupported Host Device " << device << std::endl;
  }
  host_write_ = false;
}

void RiscvCpu::Ecall() {
  int cause;
  switch (privilege_) {
    case PrivilegeMode::MACHINE_MODE:
      cause = ECALL_MMODE;
      break;
    case PrivilegeMode::SUPERVISOR_MODE:
      cause = ECALL_SMODE;
      break;
    case PrivilegeMode::USER_MODE:
    default:
      cause = ECALL_UMODE;
      break;
  }
  Trap(cause);
}

constexpr uint64_t kSstatusMask_32 = 0b1101'1110'0001'0011'0011;
constexpr uint64_t kUstatusMask_32 = 0b1101'1110'0000'0001'0001;
constexpr uint64_t kSstatusMask_64 = 0b11'0000'0000'0000'1101'1110'0001'0011'0011;
constexpr uint64_t kUstatusMask_64 = 0b00'0000'0000'0000'1101'1110'0000'0001'0001;

void RiscvCpu::ApplyMstatusToCsr() {
  const uint64_t ustatus_mask = mxl_ == 1 ? kUstatusMask_32 : kUstatusMask_64;
  const uint64_t sstatus_mask = mxl_ == 1 ? kSstatusMask_32 : kSstatusMask_64;

  csrs_[USTATUS] = mstatus_ & ustatus_mask;
  csrs_[SSTATUS] = mstatus_ & sstatus_mask;
  csrs_[MSTATUS] = mstatus_;
}

void RiscvCpu::UpdateMstatus(int16_t csr) {
  if (csr != USTATUS && csr != SSTATUS && csr != MSTATUS) {
    return;
  }
  const uint64_t ustatus_mask = mxl_ == 1 ? kUstatusMask_32 : kUstatusMask_64;
  const uint64_t sstatus_mask = mxl_ == 1 ? kSstatusMask_32 : kSstatusMask_64;
  if (csr == USTATUS) {
    const uint64_t ustatus = (csrs_[USTATUS] & ustatus_mask);
    mstatus_ = (mstatus_ & ~ustatus_mask) | ustatus;
  } else if (csr == SSTATUS) {
    const uint64_t sstatus = (csrs_[SSTATUS] & sstatus_mask);
    mstatus_ = (mstatus_ & ~sstatus_mask) | sstatus;
  } else {
    // if (csr == MSTATUS)
    mstatus_ = csrs_[MSTATUS];
  }
  // SD, FS, and XS are hardwired to zeros.
  const uint64_t sd_xs_fs_mask = 1ull << (xlen_ - 1) | 0b011110000000000000;
  mstatus_ &= ~sd_xs_fs_mask;
  // set SXLEN, MXLEN
  if (mxl_ == 2) {
    mstatus_ = (mstatus_ & ~(0b1111ull << 32)) | (0b1010ull << 32);
  }
  ApplyMstatusToCsr();
}

void RiscvCpu::DumpCpuStatus() {
  std::cout << "CSR[MSTATUS] = " << std::hex << csrs_[MSTATUS] << std::endl;
  std::cout << "PC = " << std::hex << pc_ << std::endl;
}

void RiscvCpu::DumpRegisters() {
  std::cout
    << "           X1/RA            X2/SP            X3/GP            X4/TP            "
       "X5/T0            X6/T1            X7/T2         X8/S0/FP            "
       "X9/S1           X10/A0           X11/A1           X12/A2           "
       "X13/A3           X14/A4           X15/A5           X16/A6 "
    << std::endl;
  printf("%016lx %016lx %016lx %016lx %016lx %016lx %016lx %016lx "
         "%016lx %016lx %016lx %016lx %016lx %016lx %016lx %016lx\n",
         reg_[1], reg_[2], reg_[3], reg_[4], reg_[5], reg_[6], reg_[7],
         reg_[8],
         reg_[9], reg_[10], reg_[11], reg_[12], reg_[13], reg_[14],
         reg_[15], reg_[16]
  );
  std::cout
    << "          X17/A7           X18/S2           X19/S3           X20/S4           "
       "X21/S5           X22/S6           X23/S7           X24/S8           "
       "X25/S9          X26/S10          X27/S11          X28/T3           "
       "X29/T4           X30/T5           X31/T6" << std::endl;
  printf("%016lx %016lx %016lx %016lx %016lx %016lx %016lx %016lx "
         "%016lx %016lx %016lx %016lx %016lx %016lx %016lx\n",
         reg_[17], reg_[18], reg_[19], reg_[20], reg_[21], reg_[22],
         reg_[23], reg_[24],
         reg_[25], reg_[26], reg_[27], reg_[28], reg_[29], reg_[30],
         reg_[31]
  );
}

PrivilegeMode RiscvCpu::IntToPrivilegeMode(int value) {
  switch (value) {
    case 0:
      return PrivilegeMode::USER_MODE;
    case 1:
      return PrivilegeMode::SUPERVISOR_MODE;
    case 3:
      return PrivilegeMode::MACHINE_MODE;
    default:
      throw std::out_of_range("Value " + std::to_string(value) +
                              " is not appropriate privilege level");
  }
}

void RiscvCpu::Mret() {
  uint64_t mstatus = csrs_[MSTATUS];
  // Set privilege mode to MPP.
  uint64_t mpp = bitcrop(mstatus, 2, 11);
  privilege_ = IntToPrivilegeMode(mpp);
  // Set MIE to MPIE.
  uint64_t mpie = bitcrop(mstatus, 1, 7);
  mstatus = bitset(mstatus, 1, 3, mpie);
  // Set MPIE to 1.
  uint64_t new_mpie = 1;
  mstatus = bitset(mstatus, 1, 7, new_mpie);
  // Set MPP to 0 as user mode is supported.
  uint64_t new_mpp = 0;
  mstatus = bitset(mstatus, 2, 11, new_mpp);
  csrs_[MSTATUS] = mstatus;
  next_pc_ = csrs_[MEPC];
}

void RiscvCpu::Sret() {
  uint64_t mstatus = csrs_[MSTATUS];
  // Set privilege mode to SPP.
  uint64_t spp = bitcrop(mstatus, 1, 8);
  privilege_ = IntToPrivilegeMode(spp);
  // Set SIE to SPIE.
  uint64_t spie = bitcrop(mstatus, 1, 5);
  mstatus = bitset(mstatus, 1, 1, spie);
  // Set SPIE to 1.
  uint64_t new_spie = 1;
  mstatus = bitset(mstatus, 1, 5, new_spie);
  // Set SPP to 0
  uint64_t new_spp = 0;
  mstatus = bitset(mstatus, 2, 8, new_spp);
  csrs_[MSTATUS] = mstatus;
  next_pc_ = csrs_[SEPC];
}


uint32_t RiscvCpu::GetCode(uint32_t ir) {
  uint16_t opcode = bitcrop(ir, 7, 0);
  uint8_t funct3 = bitcrop(ir, 3, 12);
  uint8_t funct7 = bitcrop(ir, 7, 25);
  uint32_t instruction = INST_ERROR;
  switch (opcode) {
    case OPCODE_ARITHLOG: // ADD, SUB
      if (funct7 == FUNC_NORM || funct7 == FUNC_ALT) {
        if (funct3 == FUNC3_ADDSUB) {
          instruction = (funct7 == FUNC_NORM) ? INST_ADD : INST_SUB;
        } else if (funct3 == FUNC3_AND) {
          instruction = INST_AND;
        } else if (funct3 == FUNC3_OR) {
          instruction = INST_OR;
        } else if (funct3 == FUNC3_XOR) {
          instruction = INST_XOR;
        } else if (funct3 == FUNC3_SR) {
          if (funct7 == FUNC_NORM) {
            instruction = INST_SRL;
          } else if (funct7 == FUNC_ALT) {
            instruction = INST_SRA;
          }
        } else if (funct3 == FUNC3_SL) {
          instruction = INST_SLL;
        } else if (funct3 == FUNC3_SLT) {
          instruction = INST_SLT;
        } else if (funct3 == FUNC3_SLTU) {
          instruction = INST_SLTU;
        }
      } else if (funct7 == FUNC_MULT) {
        if (funct3 == FUNC3_MUL) {
          instruction = INST_MUL;
        } else if (funct3 == FUNC3_MULH) {
          instruction = INST_MULH;
        } else if (funct3 == FUNC3_MULHSU) {
          instruction = INST_MULHSU;
        } else if (funct3 == FUNC3_MULHU) {
          instruction = INST_MULHU;
        } else if (funct3 == FUNC3_DIV) {
          instruction = INST_DIV;
        } else if (funct3 == FUNC3_DIVU) {
          instruction = INST_DIVU;
        } else if (funct3 == FUNC3_REM) {
          instruction = INST_REM;
        } else if (funct3 == FUNC3_REMU) {
          instruction = INST_REMU;
        }
      }
      break;
    case OPCODE_ARITHLOG_64:
      if (funct7 == FUNC_NORM || funct7 == FUNC_ALT) {
        if (funct3 == FUNC3_ADDSUB) {
          instruction = (funct7 == FUNC_NORM) ? INST_ADDW : INST_SUBW;
        } else if (funct3 == FUNC3_SL) {
          instruction = INST_SLLW;
        } else if (funct3 == FUNC3_SR) {
          if (funct7 == FUNC_NORM) {
            instruction = INST_SRLW;
          } else if (funct7 == FUNC_ALT) {
            instruction = INST_SRAW;
          }
        }
      } else if (funct7 == FUNC_MULT) {
        if (funct3 == FUNC3_MUL) {
          instruction = INST_MULW;
        } else if (funct3 == FUNC3_DIVU) {
          instruction = INST_DIVUW;
        } else if (funct3 == FUNC3_DIV) {
          instruction = INST_DIVW;
        } else if (funct3 == FUNC3_REMU) {
          instruction = INST_REMUW;
        } else if (funct3 == FUNC3_REM) {
          instruction = INST_REMW;
        }
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
        if ((funct7 >> 1) == 0b000000) {
          instruction = INST_SRLI;
        } else if ((funct7 >> 1) == 0b010000) {
          instruction = INST_SRAI;
        }
        // If top 6 bits do not match, it's an error.
      } else if (funct3 == FUNC3_SLT) {
        instruction = INST_SLTI;
      } else if (funct3 == FUNC3_SLTU) {
        instruction = INST_SLTIU;
      }
      break;
    case OPCODE_ARITHLOG_I64:
      if (funct3 == FUNC3_ADDSUB) {
        instruction = INST_ADDIW;
      } else if (funct3 == FUNC3_SL) {
        instruction = INST_SLLIW;
      } else if (funct3 == FUNC3_SR) {
        if ((funct7 >> 1) == 0b000000) {
          instruction = INST_SRLIW;
        } else if ((funct7 >> 1) == 0b010000) {
          instruction = INST_SRAIW;
        }
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
      } else if (funct3 == FUNC3_LSWU) {
        instruction = INST_LWU;
      } else if (funct3 == FUNC3_LSD) {
        instruction = INST_LD;
      }
      break;
    case OPCODE_S: // SW
      if (funct3 == FUNC3_LSB) {
        instruction = INST_SB;
      } else if (funct3 == FUNC3_LSH) {
        instruction = INST_SH;
      } else if (funct3 == FUNC3_LSW) {
        instruction = INST_SW;
      } else if (funct3 == FUNC3_LSD) {
        instruction = INST_SD;
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

} // namespace RISCV_EMULATOR
