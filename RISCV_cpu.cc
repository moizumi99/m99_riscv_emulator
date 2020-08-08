#include "RISCV_cpu.h"
#include <stdint.h>
#include <algorithm>
#include <cassert>
#include <iostream>
#include "Disassembler.h"
#include "Mmu.h"
#include "bit_tools.h"
#include "instruction_encdec.h"
#include "memory_wrapper.h"
#include "system_call_emulator.h"

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
  ClearTimerInterruptFlag();
  peripheral_ = std::make_unique<PeripheralEmulator>(mxl_);
}

RiscvCpu::RiscvCpu() : RiscvCpu(false) {}

void RiscvCpu::ClearTimerInterruptFlag() { timer_interrupt_ = false; }

void RiscvCpu::InitializeCsrs() {
  csrs_.resize(kCsrSize, 0);
  // U: User mode, S: Supervisor mode, N: User-level Interrupts, I: RV32I/RV64I
  // base ISA.
  const uint32_t extensions = 0b00000101000010000100000000;
  csrs_[MISA] = (mxl_ << (xlen_ - 2)) | extensions;
  if (mxl_ == 2) {
    // For 64 bit mode, sxl and uxl are the same as mxl.
    csrs_[MSTATUS] = (static_cast<uint64_t>(mxl_) << 34) | (static_cast<uint64_t>(mxl_) << 32);
  }
}

void RiscvCpu::DeviceInitialization() {
  if (peripheral_ == nullptr) {
    return;
  }
  peripheral_->Initialize();
}

void RiscvCpu::SetDiskImage(std::shared_ptr<std::vector<uint8_t> > disk_image) {
  assert(peripheral_ != nullptr);
  peripheral_->SetDiskImage(disk_image);
}

uint64_t RiscvCpu::VirtualToPhysical(uint64_t virtual_address, bool write_access) {
  mmu_.SetMxl(mxl_);
  PrivilegeMode privilege = privilege_;
  if (privilege == PrivilegeMode::MACHINE_MODE) {
    uint32_t mprv = bitcrop(csrs_[MSTATUS], 1, 17);
    if (mprv == 1) {
      uint32_t mpp = bitcrop(csrs_[MSTATUS], 2, 11);
      privilege = IntToPrivilegeMode(mpp);
    }
  }
  mmu_.SetPrivilege(privilege);
  uint64_t physical_address = mmu_.VirtualToPhysical(virtual_address, csrs_[SATP], write_access);
  if (mmu_.GetPageFault()) {
    page_fault_ = true;
    faulting_address_ = mmu_.GetFaultingAddress();
  }
  return physical_address;
}

// A helper function to record shift sign error.
bool RiscvCpu::CheckShiftSign(uint8_t shamt, uint8_t instruction, const std::string &message_str) {
  if (xlen_ == 32 || instruction == INST_SLLIW || instruction == INST_SRAIW || instruction == INST_SRLIW) {
    if (shamt >> 5) {
      std::cerr << message_str << " Shift value (shamt) error. shamt = " << static_cast<int>(shamt) << std::endl;
      return true;
    }
  }
  return false;
}

void RiscvCpu::SetMemory(std::shared_ptr<MemoryWrapper> memory) {
  this->memory_ = memory;
  peripheral_->SetMemory(memory);
  mmu_.SetMemory(memory);
}

uint32_t RiscvCpu::LoadCmd(uint64_t pc) {
  auto &mem = *memory_;
  uint64_t physical_address = VirtualToPhysical(pc);
  uint64_t dram_address = (physical_address >> 2) << 2;
  uint32_t cmd = mem.Read32(dram_address);
  if ((pc & 0b10) == 0b10) {
    cmd = (cmd >> 16) & 0xFFFF;
  }
  if (!page_fault_ && (pc & 0b10) == 0b10 && (cmd & 0b11) == 0b11) {
    uint64_t physical_address_upper = VirtualToPhysical(pc + 2);
    uint32_t cmd_upper = mem.Read32((physical_address_upper >> 2) << 2);
    cmd = (cmd & 0xFFFF) | (cmd_upper & 0xFFFF) << 16;
  }
  cmd = (cmd & 0b11) == 0b11 ? cmd : cmd & 0xFFFF;
  return cmd;
}

void RiscvCpu::SetRegister(uint32_t num, uint64_t value) { reg_[num] = value; }

uint64_t RiscvCpu::ReadRegister(uint32_t num) { return reg_[num]; }

void RiscvCpu::SetCsr(uint32_t index, uint64_t value) { csrs_[index] = value; }

uint64_t RiscvCpu::ReadCsr(uint32_t index) { return csrs_[index]; }

void RiscvCpu::SetWorkMemory(uint64_t top, uint64_t bottom) {
  this->top_ = top;
  this->bottom_ = bottom;
}

/* The definition of the Linux system call can be found in
 * riscv-gnu-toolchain/linux-headers/include/asm-generic/unistd.h
 */

std::pair<bool, bool> RiscvCpu::SystemCall() { return SystemCallEmulation(memory_, reg_, top_, &brk_); }

uint64_t RiscvCpu::LoadWd(uint64_t physical_address, int width) {
  assert(1 <= width && width <= 8);
  auto &mem = *memory_;
  uint64_t result = 0;
  // Mock the 32bit wide access behavior of HW.
  if (width == 4 && (physical_address & 0b11) == 0) {
    result = mem.Read32(physical_address);
  } else if (width == 8 && (physical_address & 0b111) == 0) {
    result = mem.Read64(physical_address);
  } else {
    for (int offset = 0; offset < width; ++offset) {
      uint64_t loaded_byte = mem.ReadByte(physical_address + offset);
      result |= loaded_byte << offset * 8;
    }
  }
  return result;
}

void RiscvCpu::StoreWd(uint64_t physical_address, uint64_t data, int width) {
  auto &mem = *memory_;
  assert(1 <= width && width <= 8);
  // Mock the 32bit wide access behavior of HW.
  if (width == 4 && (physical_address & 0b11) == 0) {
    mem.Write32(physical_address, data);
  } else if (width == 8 && ((physical_address & 0b111) == 0)) {
    mem.Write64(physical_address, data);
  } else {
    for (int offset = 0; offset < width; ++offset) {
      mem.WriteByte(physical_address + offset, (data >> offset * 8) & 0xFF);
    }
  }
}

void RiscvCpu::Trap(int cause, bool interrupt) {
  // Currently supported exceptions: page fault (12, 13, 15) and ecall (8, 9, 11).
  // Currently supported interrupts: Supervisor Software Interrupt (1), Machine Timer Intetrupt (7)
  assert((interrupt && (cause == MACHINE_TIMER_INTERRUPT || cause == SUPERVISOR_SOFTWARRE_INTERRUPT ||
                        cause == MACHINE_EXTERNAL_INTERRUPT)) ||
             (!interrupt &
         (cause == INSTRUCTION_PAGE_FAULT || cause == LOAD_PAGE_FAULT || cause == STORE_PAGE_FAULT ||
          cause == ECALL_UMODE || cause == ECALL_SMODE || cause == ECALL_MMODE)));
  if (interrupt && cause == MACHINE_EXTERNAL_INTERRUPT) {
    std::cerr << "Machine External Interrupt" << std::endl;
  }
  // Check the Machine Level Enable.
  const uint64_t global_mie = bitcrop(mstatus_, 1, 3);
  const uint64_t mie = csrs_[MIE];
  if (interrupt && (global_mie == 0 || bitcrop(mie, 1, cause) == 0)) {
    // Interrupt is not enabled. Don't do anything.
    return;
  }

  // Page fault specific processing.
  if (!interrupt && (cause == INSTRUCTION_PAGE_FAULT || cause == LOAD_PAGE_FAULT || cause == STORE_PAGE_FAULT)) {
    if (prev_page_fault_ && prev_faulting_address_ == faulting_address_) {
      std::cerr << "This is a page fault right after another fault at the same "
                   "address. Exit."
                << std::endl;
      error_flag_ = true;
    }
    prev_page_fault_ = page_fault_;
    prev_faulting_address_ = faulting_address_;
    page_fault_ = false;
  }


  // Check supervisor mode delegation status.
  // The original machine privilege mode must be user or supervisor mode.
  bool sv_delegation = privilege_ == PrivilegeMode::USER_MODE || privilege_ == PrivilegeMode::SUPERVISOR_MODE;

  // Check for supervisor delegation bit.
  if (interrupt) {
    const uint64_t mideleg = csrs_[MIDELEG];
    sv_delegation &= bitcrop(mideleg, 1, cause) == 1;
  } else {
    const uint64_t medeleg = csrs_[MEDELEG];
    sv_delegation &= bitcrop(medeleg, 1, cause) == 1;
  }

  // Common process for machine mode and supervisor mode.
  // MTVAL, and STVAL.
  uint64_t tval = 0;
  if (!interrupt) {
    if (cause == INSTRUCTION_PAGE_FAULT || cause == LOAD_PAGE_FAULT || cause == STORE_PAGE_FAULT) {
      tval = faulting_address_;
    } else if (cause == ILLEGAL_INSTRUCTION) {
      tval = ir_;
    }
  }
  const uint64_t interrupt_cause_mask = interrupt ? 1 << (xlen_ - 1) : 0;

  // Processing.
  // TODO: Add user delegation.
  if (sv_delegation) {
    // Check Supervisor Interrupt Enable.
    const uint64_t global_sie = bitcrop(mstatus_, 1, 1);
    if (interrupt && (global_sie == 0 || bitcrop(csrs_[SIE], 1, cause) == 0)) {
      return;
    }
    csrs_[SCAUSE] = cause | interrupt_cause_mask;
    csrs_[SEPC] = pc_;
    const uint64_t tvec = csrs_[STVEC];
    const uint8_t mode = tvec & 0b11;
    if (mode == 0 || interrupt) {
      next_pc_ = tvec & ~0b11;
    } else {
      next_pc_ = (tvec & ~0b11) + 4 * (csrs_[SCAUSE] & GenMask<uint64_t>(xlen_ - 1, 0));
    }
    csrs_[STVAL] = tval;

    // Copy old SIE (#1) to SPIE (#5). Then, disable SIE (#1).
    mstatus_ = bitset(mstatus_, 1, 5, global_sie);
    constexpr uint64_t kNewSie = 0;
    mstatus_ = bitset(csrs_[SSTATUS], 1, 1, kNewSie);

    // Save the current privilege mode to SPP (#12-11), and set the privilege to
    // Supervisor.
    const uint64_t new_spp = static_cast<int>(privilege_) & 1;
    mstatus_ = bitset(mstatus_, 1, 8, new_spp);
    privilege_ = PrivilegeMode::SUPERVISOR_MODE;
  } else {
    csrs_[MCAUSE] = cause | interrupt_cause_mask;
    csrs_[MEPC] = pc_;
    const uint64_t tvec = csrs_[MTVEC];
    const uint8_t mode = tvec & 0b11;
    if (mode == 0 || interrupt) {
      next_pc_ = tvec & ~0b11;
    } else {
      next_pc_ = (tvec & ~0b11) + 4 * (csrs_[MCAUSE] & GenMask<uint64_t>(xlen_ - 1, 0));
    }
    // Store the faulting address to MTVAL.
    csrs_[MTVAL] = tval;

    // Copy old MIE (#3) to MPIE (#7). Then, disable MIE.
    mstatus_ = bitset(mstatus_, 1, 7, global_mie);
    constexpr uint64_t kNewMie = 0;
    mstatus_ = bitset(mstatus_, 1, 3, kNewMie);

    // Save the current privilege mode to MPP (#12-11), and set the privilege to
    // Machine.
    const uint64_t new_mpp = static_cast<int>(privilege_);
    mstatus_ = bitset(mstatus_, 2, 11, new_mpp);
    // Clear interrupt pending bit.
    privilege_ = PrivilegeMode::MACHINE_MODE;
  }
  // Clear interrupt pending bit.
  if (interrupt) {
    ClearInterruptPending(cause);
  }
  ApplyMstatusToCsr();
}

void RiscvCpu::ClearInterruptPending(int cause) {
  constexpr uint64_t sip_mask = 0b001100110011;
  constexpr uint64_t uip_mask = 0b000100010001;
  uint64_t mip = csrs_[MIP];
  bitset(mip, 1, cause, static_cast<uint64_t>(0));
  csrs_[MIP] = mip;
  csrs_[SIP] = (csrs_[SIP] & ~sip_mask) | (mip & sip_mask);
  csrs_[UIP] = (csrs_[UIP] & ~uip_mask) | (mip & uip_mask);
  return;
}

uint64_t kUpper32bitMask = 0xFFFFFFFF00000000;

uint64_t RiscvCpu::Sext32bit(uint64_t data32bit) {
  if ((data32bit >> 31) & 1) {
    return data32bit | kUpper32bitMask;
  } else {
    return data32bit & (~kUpper32bitMask);
  }
}

void RiscvCpu::CsrsInstruction(uint32_t instruction, uint32_t csr, uint32_t rd, uint32_t rs1) {
  uint64_t t = csrs_[csr];
  uint64_t new_t = t;
  switch (instruction) {
    case INST_CSRRC:
      new_t &= ~reg_[rs1];
      break;
    case INST_CSRRCI:
      new_t &= ~rs1;
      break;
    case INST_CSRRS:
      new_t |= reg_[rs1];
      break;
    case INST_CSRRSI:
      new_t |= rs1;
      break;
    case INST_CSRRW:
      new_t = reg_[rs1];
      break;
    case INST_CSRRWI:
      new_t = rs1;
      break;
    default:;
  }
  // TODO: Check for privilege before writing back to CSR.
  csrs_[csr] = new_t;
  reg_[rd] = t;
  UpdateStatus(csr);
}

uint64_t RiscvCpu::BranchInstruction(uint32_t instruction, uint32_t rs1, uint32_t rs2, int32_t imm13) {
  bool condition = false;
  switch (instruction) {
    case INST_BEQ:
      condition = reg_[rs1] == reg_[rs2];
      break;
    case INST_BGE:
      condition = static_cast<int64_t>(reg_[rs1]) >= static_cast<int64_t>(reg_[rs2]);
      break;
    case INST_BGEU:
      condition = reg_[rs1] >= reg_[rs2];
      break;
    case INST_BLT:
      condition = static_cast<int64_t>(reg_[rs1]) < static_cast<int64_t>(reg_[rs2]);
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

void RiscvCpu::OperationInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1, uint32_t rs2) {
  bool w_instruction = instruction == INST_ADDW || instruction == INST_SUBW || instruction == INST_SLLW ||
                       instruction == INST_SRLW || instruction == INST_SRAW;
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
    default:
      temp64 = 0;
      std::cerr << "Undefined Arithmetic or Logical instruction detected." << std::endl;
      assert(false);
  }
  if (xlen_ == 32 || w_instruction) {
    temp64 = Sext32bit(temp64);
  }
  reg_[rd] = temp64;
}

void RiscvCpu::ImmediateInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1, int32_t imm12) {
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
    default:
      temp64 = 0;
      std::cerr << "Unsupported Immediate instruction detected." << std::endl;
      assert(false);
  }
  if (xlen_ == 32 || instruction == INST_ADDIW) {
    temp64 = Sext32bit(temp64);
  }
  reg_[rd] = temp64;
}

void RiscvCpu::ImmediateShiftInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1, uint32_t shamt) {
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

int RiscvCpu::GetLoadWidth(uint32_t instruction) {
  int width = 8;
  switch(instruction) {
    case INST_LB:
    case INST_LBU:
      width = 1;
      break;
    case INST_LH:
    case INST_LHU:
      width = 2;
      break;
    case INST_LW:
    case INST_LWU:
      width = 4;
      break;
    case INST_LD:
      width = 8;
      break;
    default:
      assert(false);
  }
  return width;
}

void RiscvCpu::LoadInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1, int32_t imm12) {
  peripheral_->MemoryMappedValueUpdate();
  uint64_t source_address = reg_[rs1] + imm12;
  uint64_t address = VirtualToPhysical(source_address);
  if (page_fault_) {
    Trap(ExceptionCode::LOAD_PAGE_FAULT, kException);
    return;
  }
  int width = GetLoadWidth(instruction);
  int access_width = GetAccessWidth(width, address);
  int next_width = width - access_width;
  uint64_t load_data = LoadWd(address, access_width);
  if (next_width > 0) {
    uint64_t next_address = VirtualToPhysical(address + access_width, false);
    if (page_fault_) {
      Trap(ExceptionCode::LOAD_PAGE_FAULT, kException);
      return;
    }
    uint64_t load_data_high = LoadWd(next_address, next_width);
    load_data |= (load_data_high << access_width * 8);
  }
  if (instruction == INST_LB || instruction == INST_LH || instruction == INST_LW) {
    load_data = SignExtend(load_data, width * 8);
  } else if (instruction == INST_LWU) {
    load_data &= 0xFFFFFFFF;
  }

  reg_[rd] = load_data;
}

int RiscvCpu::GetStoreWidth(uint32_t instruction) {
  int width = 8;
  switch(instruction) {
    case INST_SB:
      width = 1;
      break;
    case INST_SH:
      width = 2;
      break;
    case INST_SW:
      width = 4;
      break;
    case INST_SD:
      width = 8;
      break;
    default:
      assert(false);
  }
  return width;
}

int RiscvCpu::GetAccessWidth(uint32_t width, uint64_t address) {
  bool burst_overflow = ((address & 0b111) + width - 1) >> 3;
  int access_width = width;
  if (burst_overflow) {
    const int mask = width == 2 ? 0b1 : (width == 4 ? 0b11 : (width == 8 ? 0b111 : 0));
    access_width = width - (address & mask);
  }
  return access_width;
}

void RiscvCpu::StoreInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1, uint32_t rs2, int32_t imm12_stype) {
  uint64_t dst_address = reg_[rs1] + imm12_stype;
  // Check if the access crosses memory access unit (64 bit).
  uint64_t address = VirtualToPhysical(dst_address, true);
  if (page_fault_) {
    Trap(ExceptionCode::STORE_PAGE_FAULT, kException);
    return;
  }
  int width = GetStoreWidth(instruction);
  int access_width = GetAccessWidth(width, dst_address);
  int next_width = width - access_width;
  int64_t data = reg_[rs2] & GenerateBitMask(access_width * 8);
  StoreWd(address, data, access_width);
  if (next_width > 0) {
    uint64_t next_address = VirtualToPhysical(dst_address + access_width, true);
    if (page_fault_) {
      Trap(ExceptionCode::STORE_PAGE_FAULT, kException);
      return;
    }
    uint64_t next_data = reg_[rs2] >> (access_width * 8);
    StoreWd(next_address, next_data, next_width);
  }
  peripheral_->CheckDeviceWrite(address, width, reg_[rs2]);
}

void RiscvCpu::SystemInstruction(uint32_t instruction, uint32_t rd, int32_t imm) {
  if (imm == 0b000000000000) {
    // ECALL
    if (ecall_emulation_) {
      std::tie(error_flag_, end_flag_) = SystemCall();
    } else {
      Ecall();
    }
  } else if (imm == 0b000000000001) {
    // EBREAK
    // Debug function is not implemented yet.
  } else if (imm == 0b001100000010) {
    Mret();
  } else if (imm == 0b000100000010) {
    Sret();
  } else if (((imm >> 5) == 0b0001001) && (rd == 0b00000)) {
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

void RiscvCpu::MultInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1, uint32_t rs2) {
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
        temp64 = (static_cast<int64_t>(reg_[rs1]) * static_cast<int64_t>(reg_[rs2])) >> 32;
      } else {
        temp64 = GetMulh64(reg_[rs1], reg_[rs2]);
      }
      break;
    case INST_MULHSU:
      if (xlen_ == 32) {
        temp64 = (static_cast<int64_t>(reg_[rs1]) * (reg_[rs2] & 0xFFFFFFFF)) >> 32;
      } else {
        temp64 = GetMulhsu64(static_cast<int64_t>(reg_[rs1]), reg_[rs2]);
      }
      break;
    case INST_MULHU:
      if (xlen_ == 32) {
        temp64 = ((reg_[rs1] & 0xFFFFFFFF) * (reg_[rs2] & 0xFFFFFFFF)) >> 32;
      } else {
        temp64 = GetMulhu64(reg_[rs1], reg_[rs2]);
      }
      break;
    case INST_DIV:
      if (reg_[rs2] == 0) {
        temp64 = -1;
      } else if (static_cast<int64_t>(reg_[rs2]) == -1 && reg_[rs1] == largest_negative) {
        // Overflow check.
        temp64 = largest_negative;
      } else {
        temp64 = static_cast<int64_t>(reg_[rs1]) / static_cast<int64_t>(reg_[rs2]);
      }
      break;
    case INST_DIVU:
      if (reg_[rs2] == 0) {
        temp64 = ~0;
      } else if (xlen_ == 32) {
        temp64 = static_cast<uint64_t>(reg_[rs1] & 0xFFFFFFFF) / static_cast<uint64_t>(reg_[rs2] & 0xFFFFFFFF);
      } else {
        temp64 = static_cast<uint64_t>(reg_[rs1]) / static_cast<uint64_t>(reg_[rs2]);
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
      } else if (static_cast<int64_t>(reg_[rs2]) == -1 && reg_[rs1] == (1lu << 31)) {
        // Overflow check.
        temp64 = 1 << 31;
      } else {
        temp64 = static_cast<int64_t>(SignExtend(reg_[rs1], 32)) / static_cast<int64_t>(SignExtend(reg_[rs2], 32));
      }
      sign_extend_en = true;
      break;
    case INST_REM:
      if (reg_[rs2] == 0) {
        temp64 = reg_[rs1];
      } else if (static_cast<int64_t>(reg_[rs2]) == -1 && reg_[rs1] == largest_negative) {
        // Overflow check.
        temp64 = 0;
      } else {
        temp64 = static_cast<int64_t>(reg_[rs1]) % static_cast<int64_t>(reg_[rs2]);
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
      } else if (static_cast<int64_t>(reg_[rs2]) == -1 && reg_[rs1] == (1lu << 31)) {
        // Overflow check.
        temp64 = 0;
      } else {
        temp64 = static_cast<int64_t>(SignExtend(reg_[rs1], 32)) % static_cast<int64_t>(SignExtend(reg_[rs2], 32));
      }
      sign_extend_en = true;
      break;
    default:
      temp64 = 0;
      std::cerr << "Undefined Mul instruction detected." << std::endl;
      assert(false);
  }
  if (sign_extend_en) {
    temp64 = Sext32bit(temp64);
  }
  reg_[rd] = temp64;
}

template <class T>
T max(T a, T b) {
  return a > b ? a : b;
}

template <class T>
T min(T a, T b) {
  return a < b ? a : b;
}

void RiscvCpu::AmoInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1, uint32_t rs2) {
  uint64_t virtual_address = reg_[rs1];
  uint64_t physical_address = VirtualToPhysical(virtual_address);
  uint64_t new_value;
  uint32_t word_width = (instruction == INST_AMOADDD || instruction == INST_AMOANDD || instruction == INST_AMOMAXD ||
                         instruction == INST_AMOMAXUD || instruction == INST_AMOMIND || instruction == INST_AMOMINUD ||
                         instruction == INST_AMOORD || instruction == INST_AMOXORD || instruction == INST_AMOSWAPD)
                            ? 8
                            : 4;
  uint64_t t = LoadWd(physical_address, word_width);
  if (word_width == 4) {
    t = SignExtend(t, 32);
  }
  switch (instruction) {
    case INST_AMOADDD:
      new_value = t + reg_[rs2];
      break;
    case INST_AMOADDW:
      new_value = SignExtend(t + reg_[rs2], 32);
      break;
    case INST_AMOANDD:
      new_value = t & reg_[rs2];
      break;
    case INST_AMOANDW:
      new_value = SignExtend(t & reg_[rs2], 32);
      break;
    case INST_AMOMAXD:
      new_value = max<int64_t>(t, reg_[rs2]);
      break;
    case INST_AMOMAXW:
      new_value = max<int32_t>(SignExtend(t, 32), SignExtend(reg_[rs2], 32));
      new_value = SignExtend(new_value, 32);
      break;
    case INST_AMOMAXUD:
      new_value = max<uint64_t>(t, reg_[rs2]);
      break;
    case INST_AMOMAXUW:
      new_value = max<uint32_t>(static_cast<uint32_t>(t), static_cast<uint32_t>(reg_[rs2]));
      new_value = SignExtend(new_value, 32);
      break;
    case INST_AMOMIND:
      new_value = min<int64_t>(t, reg_[rs2]);
      break;
    case INST_AMOMINW:
      new_value = min<int32_t>(SignExtend(t, 32), SignExtend(reg_[rs2], 32));
      new_value = SignExtend(new_value, 32);
      break;
    case INST_AMOMINUD:
      new_value = min<uint64_t>(t, reg_[rs2]);
      break;
    case INST_AMOMINUW:
      new_value = min<uint32_t>(static_cast<uint32_t>(t), static_cast<uint32_t>(reg_[rs2]));
      new_value = SignExtend(new_value, 32);
      break;
    case INST_AMOORD:
      new_value = t | reg_[rs2];
      break;
    case INST_AMOORW:
      new_value = SignExtend(t | reg_[rs2], 32);
      break;
    case INST_AMOXORD:
      new_value = t ^ reg_[rs2];
      break;
    case INST_AMOXORW:
      new_value = SignExtend(t ^ reg_[rs2], 32);
      break;
    case INST_AMOSWAPD:
    case INST_AMOSWAPW:
      new_value = reg_[rs2];
      break;
    default:
      new_value = 0;
      std::cerr << "Undefined AMOxxx instruction." << std::endl;
      assert(false);
  }
  constexpr bool kWriteAccess = true;
  physical_address = VirtualToPhysical(virtual_address, kWriteAccess);
  StoreWd(physical_address, new_value, word_width);
  reg_[rd] = t;
}

bool RiscvCpu::TimerTick() {
  peripheral_->TimerTick();
  if (!peripheral_->GetTimerInterrupt()) {
    return false;
  }
  peripheral_->ClearTimerInterrupt();
  Trap(ExceptionCode::MACHINE_TIMER_INTERRUPT, kInterrupt);
  return true;
}

void RiscvCpu::DumpDisassembly(bool verbose) {
  if (!verbose) {
    return;
  }
  char machine_status =
      privilege_ == PrivilegeMode::USER_MODE ? 'U' : privilege_ == PrivilegeMode::SUPERVISOR_MODE ? 'S' : 'M';
  printf("%c %016lx (%04x): ", machine_status, pc_, ir_);
  std::cout << Disassemble(ir_, mxl_) << std::endl;
}

int RiscvCpu::RunCpu(uint64_t start_pc, bool verbose) {
  error_flag_ = false;
  end_flag_ = false;

  next_pc_ = start_pc;
  do {
    pc_ = next_pc_;
    if (TimerTick()) {
      continue;
    }
    CheckSoftwareInterrupt();
    ir_ = LoadCmd(pc_);
    DumpDisassembly(verbose);
    if (page_fault_) {
      Trap(ExceptionCode::INSTRUCTION_PAGE_FAULT, kException);
      continue;
    }
    if (virtio_interrupt_) {
      Trap(ExceptionCode::MACHINE_EXTERNAL_INTERRUPT, kInterrupt);
      virtio_interrupt_ = false;
      continue;
    }
    // Decode. Mimick the HW behavior. (In HW, decode is in parallel.)
    ctype_ = (ir_ & 0b11) != 0b11;

    uint32_t instruction, rd, rs1, rs2;
    int32_t imm;
    int16_t csr = GetCsr(ir_);
    if (!ctype_) {
      next_pc_ = pc_ + 4;
      instruction = GetCode32(ir_);
      rd = GetRd(ir_);
      rs1 = GetRs1(ir_);
      rs2 = GetRs2(ir_);
      imm = GetImm(ir_);
    } else {
      next_pc_ = pc_ + 2;
      GetCode16(ir_, mxl_, &instruction, &rd, &rs1, &rs2, &imm);
    }
    uint64_t t;  // 't' is used in RISCV Reader to show a temporary address.
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
        ImmediateInstruction(instruction, rd, rs1, imm);
        break;
      case INST_SLLI:
      case INST_SLLIW:
      case INST_SRLI:
      case INST_SRLIW:
      case INST_SRAI:
      case INST_SRAIW:
        ImmediateShiftInstruction(instruction, rd, rs1, imm);
        break;
      case INST_SLT:
        reg_[rd] = (static_cast<int64_t>(reg_[rs1]) < static_cast<int64_t>(reg_[rs2])) ? 1 : 0;
        break;
      case INST_SLTU:
        reg_[rd] = (reg_[rs1] < reg_[rs2]) ? 1 : 0;
        break;
      case INST_SLTI:
        reg_[rd] = static_cast<int64_t>(reg_[rs1]) < imm ? 1 : 0;
        break;
      case INST_SLTIU:
        reg_[rd] = reg_[rs1] < static_cast<uint64_t>(imm) ? 1 : 0;
        break;
      case INST_BEQ:
      case INST_BGE:
      case INST_BGEU:
      case INST_BLT:
      case INST_BLTU:
      case INST_BNE:
        next_pc_ = BranchInstruction(instruction, rs1, rs2, imm);
        break;
      case INST_JAL:
        reg_[rd] = next_pc_;
        next_pc_ = pc_ + imm;
        if (next_pc_ == pc_) {
          error_flag_ = true;
        }
        break;
      case INST_JALR:
        t = next_pc_;
        next_pc_ = (reg_[rs1] + imm) & ~1;
        reg_[rd] = t;
        // Below lines are only for simulation purpose.
        // Remove once a better solution is found.
        if (rd == ZERO && rs1 == RA && reg_[rs1] == 0 && imm == 0) {
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
        LoadInstruction(instruction, rd, rs1, imm);
        break;
      case INST_SB:
      case INST_SH:
      case INST_SW:
      case INST_SD:
        StoreInstruction(instruction, rd, rs1, rs2, imm);
        break;
      case INST_LUI:
        temp64 = imm;
        if (xlen_ == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_AUIPC:
        temp64 = pc_ + imm;
        if (xlen_ == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_SYSTEM:
        SystemInstruction(instruction, rd, imm);
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
      case INST_AMOADDD:
      case INST_AMOADDW:
      case INST_AMOANDD:
      case INST_AMOANDW:
      case INST_AMOMAXD:
      case INST_AMOMAXW:
      case INST_AMOMAXUD:
      case INST_AMOMAXUW:
      case INST_AMOMIND:
      case INST_AMOMINW:
      case INST_AMOMINUD:
      case INST_AMOMINUW:
      case INST_AMOORD:
      case INST_AMOORW:
      case INST_AMOXORD:
      case INST_AMOXORW:
      case INST_AMOSWAPD:
      case INST_AMOSWAPW:
        AmoInstruction(instruction, rd, rs1, rs2);
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
    if (host_emulation_ || peripheral_emulation_) {
      PeripheralEmulations();
    }

  } while (!error_flag_ && !end_flag_);

  if (error_flag_ && verbose) {
    DumpCpuStatus();
  }
  return error_flag_;
}

void RiscvCpu::CheckSoftwareInterrupt() {
  uint64_t interrupt_status = csrs_[CsrsAddresses::MIP] & csrs_[CsrsAddresses::MIE];
  bool interrupt_enable = bitcrop(mstatus_, 1, 3) == 1;
  if (!interrupt_enable || interrupt_status == 0) {
    return;
  }
  if (bitcrop(interrupt_status, 1, 3) == 1) {
    Trap(ExceptionCode::MACHINE_SOFTWARE_INTERRUPT, kInterrupt);
  } else if ( bitcrop(interrupt_status, 1, 1) == 1) {
    Trap(ExceptionCode::SUPERVISOR_SOFTWARRE_INTERRUPT, kInterrupt);
  } else if (bitcrop(interrupt_status, 1, 0) == 1) {
    Trap(ExceptionCode::USER_SOFTWARE_INTERRUPT, kInterrupt);
  }
}

// Peripheral emulation needed for XV6 and RISCV-TEST.
// reference: https://github.com/riscv/riscv-isa-sim/issues/364
void RiscvCpu::PeripheralEmulations() {
  peripheral_->Emulation();
  if (peripheral_->GetHostEndFlag()) {
    reg_[A0] = peripheral_->GetHostValue();
    error_flag_ |= peripheral_->GetHostErrorFlag();
    end_flag_ = true;
  }
  if (peripheral_->GetInterruptStatus()) {
    peripheral_->ClearInterruptStatus();
    virtio_interrupt_ = true;
  }
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
  Trap(cause, kException);
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

template <class T, int N>
bool in(T obj, std::array<T, N> target) {
  return (std::end(target) != std::find(std::begin(target), std::end(target), obj));
}

void RiscvCpu::UpdateStatus(int16_t csr) {
  if (in<int16_t, 3>(csr, {USTATUS, SSTATUS, MSTATUS})) {
    UpdateMstatus(csr);
  } else if (in<int16_t, 3>(csr, {UIP, SIP, MIP})) {
    UpdateInterruptPending(csr);
  } else if (in<int16_t, 3>(csr, {UIE, SIE, MIE})) {
    UpdateInterruptEnable(csr);
  }
}

void RiscvCpu::UpdateMstatus(int16_t csr) {
  const uint64_t ustatus_mask = mxl_ == 1 ? kUstatusMask_32 : kUstatusMask_64;
  const uint64_t sstatus_mask = mxl_ == 1 ? kSstatusMask_32 : kSstatusMask_64;
  if (csr == USTATUS) {
    const uint64_t ustatus = (csrs_[USTATUS] & ustatus_mask);
    mstatus_ = (mstatus_ & ~ustatus_mask) | ustatus;
  } else if (csr == SSTATUS) {
    const uint64_t sstatus = (csrs_[SSTATUS] & sstatus_mask);
    mstatus_ = (mstatus_ & ~sstatus_mask) | sstatus;
  } else if (csr == MSTATUS) {
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

void RiscvCpu::UpdateInterruptPending(int16_t csr) {
  constexpr uint64_t kUipMask = 0b0000100010001;
  constexpr uint64_t kSipMask = 0b0001100110011;
  uint64_t mip = csrs_[CsrsAddresses::MIP];
  if (csr == CsrsAddresses::UIP) {
    uint64_t uip = csrs_[CsrsAddresses::UIP] & kUipMask;
    mip = (mip & ~kUipMask) | uip;
  } else if (csr == CsrsAddresses::SIP) {
    uint64_t sip = csrs_[CsrsAddresses::SIP] & kSipMask;
    mip = (mip & ~kSipMask) | sip;
  }
  csrs_[CsrsAddresses::UIP] = mip & kUipMask;
  csrs_[CsrsAddresses::SIP] = mip & kSipMask;
  csrs_[CsrsAddresses::MIP] = mip;
}

void RiscvCpu::UpdateInterruptEnable(int16_t csr) {
  constexpr uint64_t kUieMask = 0b0000100010001;
  constexpr uint64_t kSieMask = 0b0001100110011;
  uint64_t mie = csrs_[CsrsAddresses::MIE];
  if (csr == CsrsAddresses::UIE) {
    uint64_t uie = csrs_[CsrsAddresses::UIE] & kUieMask;
    mie = (mie & ~kUieMask) | uie;
  } else if (csr == CsrsAddresses::SIE) {
    uint64_t sie = csrs_[CsrsAddresses::SIE] & kSieMask;
    mie = (mie & ~kSieMask) | sie;
  }
  csrs_[CsrsAddresses::UIE] = mie & kUieMask;
  csrs_[CsrsAddresses::SIE] = mie & kSieMask;
  csrs_[CsrsAddresses::MIE] = mie;
}

void RiscvCpu::DumpCpuStatus() {
  std::cout << "CSR[MSTATUS] = " << std::hex << csrs_[MSTATUS] << std::endl;
  std::cout << "PC = " << std::hex << pc_ << std::endl;
}

void RiscvCpu::DumpRegisters() {
  std::cout << "           X1/RA            X2/SP            X3/GP            X4/TP  "
               "          "
               "X5/T0            X6/T1            X7/T2         X8/S0/FP            "
               "X9/S1           X10/A0           X11/A1           X12/A2           "
               "X13/A3           X14/A4           X15/A5           X16/A6 "
            << std::endl;
  printf(
      "%016lx %016lx %016lx %016lx %016lx %016lx %016lx %016lx "
      "%016lx %016lx %016lx %016lx %016lx %016lx %016lx %016lx\n",
      reg_[1], reg_[2], reg_[3], reg_[4], reg_[5], reg_[6], reg_[7], reg_[8], reg_[9], reg_[10], reg_[11], reg_[12],
      reg_[13], reg_[14], reg_[15], reg_[16]);
  std::cout << "          X17/A7           X18/S2           X19/S3           X20/S4  "
               "         "
               "X21/S5           X22/S6           X23/S7           X24/S8           "
               "X25/S9          X26/S10          X27/S11          X28/T3           "
               "X29/T4           X30/T5           X31/T6"
            << std::endl;
  printf(
      "%016lx %016lx %016lx %016lx %016lx %016lx %016lx %016lx "
      "%016lx %016lx %016lx %016lx %016lx %016lx %016lx\n",
      reg_[17], reg_[18], reg_[19], reg_[20], reg_[21], reg_[22], reg_[23], reg_[24], reg_[25], reg_[26], reg_[27],
      reg_[28], reg_[29], reg_[30], reg_[31]);
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
      throw std::out_of_range("Value " + std::to_string(value) + " is not appropriate privilege level");
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

uint32_t RiscvCpu::GetCode32(uint32_t ir) {
  uint16_t opcode = bitcrop(ir, 7, 0);
  uint8_t funct3 = bitcrop(ir, 3, 12);
  uint8_t funct7 = bitcrop(ir, 7, 25);
  uint8_t funct5 = funct7 >> 2;
  uint32_t instruction = INST_ERROR;
  switch (opcode) {
    case OPCODE_ARITHLOG:  // ADD, SUB
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
    case OPCODE_ARITHLOG_I:  // ADDI, SUBI
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
    case OPCODE_B:  // beq, bltu, bge, bne
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
    case OPCODE_J:  // jal
      instruction = INST_JAL;
      break;
    case OPCODE_JALR:  // jalr
      if (funct3 == FUNC3_JALR) {
        instruction = INST_JALR;
      }
      break;
    case OPCODE_LD:  // LW
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
    case OPCODE_S:  // SW
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
    case OPCODE_LUI:  // LUI
      instruction = INST_LUI;
      break;
    case OPCODE_AUIPC:  // AUIPC
      instruction = INST_AUIPC;
      break;
    case OPCODE_SYSTEM:  // System instructions.
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
    case OPCODE_AMO:
      if (funct5 == FUNC5_AMOADD) {
        instruction = funct3 == FUNC3_AMOD ? INST_AMOADDD : INST_AMOADDW;
      } else if (funct5 == FUNC5_AMOAND) {
        instruction = funct3 == FUNC3_AMOD ? INST_AMOANDD : INST_AMOANDW;
      } else if (funct5 == FUNC5_AMOMAX) {
        instruction = funct3 == FUNC3_AMOD ? INST_AMOMAXD : INST_AMOMAXW;
      } else if (funct5 == FUNC5_AMOMAXU) {
        instruction = funct3 == FUNC3_AMOD ? INST_AMOMAXUD : INST_AMOMAXUW;
      } else if (funct5 == FUNC5_AMOMIN) {
        instruction = funct3 == FUNC3_AMOD ? INST_AMOMIND : INST_AMOMINW;
      } else if (funct5 == FUNC5_AMOMINU) {
        instruction = funct3 == FUNC3_AMOD ? INST_AMOMINUD : INST_AMOMINUW;
      } else if (funct5 == FUNC5_AMOOR) {
        instruction = funct3 == FUNC3_AMOD ? INST_AMOORD : INST_AMOORW;
      } else if (funct5 == FUNC5_AMOXOR) {
        instruction = funct3 == FUNC3_AMOD ? INST_AMOXORD : INST_AMOXORW;
      } else if (funct5 == FUNC5_AMOSWAP) {
        instruction = funct3 == FUNC3_AMOD ? INST_AMOSWAPD : INST_AMOSWAPW;
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

void RiscvCpu::GetCode16(uint32_t ir, int mxl, uint32_t *instruction_out,
                         uint32_t *rd_out, uint32_t *rs1_out, uint32_t *rs2_out, int32_t *imm_out) {
  uint32_t opcode = (((ir >> 13) & 0b111) << 2) | (ir & 0b11);
  uint32_t instruction = INST_ERROR;
  uint32_t rd = 0, rs1 = 0, rs2 = 0;
  int32_t imm = 0;
  switch (opcode) {
    case 0b00000:
      if (bitcrop(ir, 8, 5) == 0) {
        std::cerr << "uimm must not be zero for c.addi4spn." << std::endl;
        break;
      }
      instruction = INST_ADDI;
      rs1 = 2;
      rd = bitcrop(ir, 3, 2) + 8;
      imm = bitcrop(ir, 2, 11) << 4;
      imm |= bitcrop(ir, 4, 7) << 6;
      imm |= bitcrop(ir, 1, 6) << 2;
      imm |= bitcrop(ir, 1, 5) << 3;
      break;
    case 0b00001:
    case 0b00101:
      if (opcode == 0b00101 && mxl == 1) {
        instruction = INST_JAL;
        rd = X1;
        imm = bitcrop(ir, 1, 12) << 11;
        imm |= bitcrop(ir, 1, 11) << 4;
        imm |= bitcrop(ir, 2, 9) << 8;
        imm |= bitcrop(ir, 1, 8) << 10;
        imm |= bitcrop(ir, 1, 7) << 6;
        imm |= bitcrop(ir, 1, 6) << 7;
        imm |= bitcrop(ir, 3, 3) << 1;
        imm |= bitcrop(ir, 1, 2) << 5;
        imm = SignExtend(imm, 12);
        break;
      }
      instruction = (opcode >> 2) == 0 ? INST_ADDI : INST_ADDIW;
      rs1 = rd = bitcrop(ir, 5, 7);
      imm = bitcrop(ir, 1, 12) << 5;
      imm |= bitcrop(ir, 5, 2);
      imm = SignExtend(imm, 6);
      break;
    case 0b00010:
      instruction = INST_SLLI;
      rs1 = rd = bitcrop(ir, 5, 7);
      imm = bitcrop(ir, 1, 12) << 5 | bitcrop(ir, 5, 2);
      break;
    case 0b01000:
      instruction = INST_LW;
      rs1 = bitcrop(ir, 3, 7) + 8;
      rd = bitcrop(ir, 3, 2) + 8;
      imm = bitcrop(ir, 3, 10) << 3;
      imm |= bitcrop(ir, 1, 6) << 2;
      imm |= bitcrop(ir, 1, 5) << 6;
      break;
    case 0b01001:
      instruction = INST_ADDI;
      rs1 = 0;
      rd = bitcrop(ir, 5, 7);
      imm = bitcrop(ir, 1, 12) << 5;
      imm |= bitcrop(ir, 5, 2);
      imm = SignExtend(imm, 6);
      break;
    case 0b01010:
      instruction = INST_LW;
      rd = bitcrop(ir, 5, 7);
      rs1 = X2;
      imm = bitcrop(ir, 1, 12) << 5;
      imm |= bitcrop(ir, 3, 4) << 2;
      imm |= bitcrop(ir, 2, 2) << 6;
      break;
    case 0b01100:
      instruction = INST_LD;
      rd = bitcrop(ir, 3, 2) + 8;
      rs1 = bitcrop(ir, 3, 7) + 8;
      imm = bitcrop(ir, 3, 10) << 3 | bitcrop(ir, 2, 5) << 6;
      break;
    case 0b01110:
      instruction = INST_LD;
      rd = bitcrop(ir, 5, 7);
      rs1 = 2;
      imm = bitcrop(ir, 1, 12) << 5;
      imm |= bitcrop(ir, 2, 5) << 3;
      imm |= bitcrop(ir, 3, 2) << 6;
      break;
    case 0b01101:
      rd = bitcrop(ir, 5, 7);
      if (rd != X2) {
        instruction = INST_LUI;
        imm = bitcrop(ir, 1, 12) << 17;
        imm |= bitcrop(ir, 5, 2) << 12;
        imm = SignExtend(imm, 18);
        if (imm == 0) {
          // Invalid if imm is 0.
          instruction = INST_ERROR;
        }
        break;
      } else {
        instruction = INST_ADDI;
        rs1 = 2;
        imm = bitcrop(ir, 1, 12) << 9;
        imm |= bitcrop(ir, 1, 6) << 4;
        imm |= bitcrop(ir, 1, 5) << 6;
        imm |= bitcrop(ir, 2, 3) << 7;
        imm |= bitcrop(ir, 1, 2) << 5;
        imm = SignExtend(imm, 10);
        if (imm == 0) {
          // Invalid if imm is 0.
          instruction = INST_ERROR;
        }
      }
      break;
    case 0b10010:  // c.add.
      if (bitcrop(ir, 1, 12) == 1) {
        if (bitcrop(ir, 5, 2) == 0 && bitcrop(ir, 5, 7) == 0) {
          // c.ebreak.
          instruction = INST_SYSTEM;
          imm = 1;
          break;
        } else if (bitcrop(ir, 5, 2) == 0) {
          // c.jalr.
          instruction = INST_JALR;
          rs1 = bitcrop(ir, 5, 7);
          rd = X1;
          imm = 0;
          break;
        }
        if (bitcrop(ir, 5, 7) == 0) {
          // invalid.
          break;
        }
        // c.add.
        instruction = INST_ADD;
        rs1 = rd = bitcrop(ir, 5, 7);
        rs2 = bitcrop(ir, 5, 2);
      } else {
        if (bitcrop(ir, 5, 2) == 0) {
          instruction = INST_JALR;
          rs1 = bitcrop(ir, 5, 7);
          rd = 0;
          imm = 0;
        } else {
          // c.mv.
          instruction = INST_ADD;
          rd = bitcrop(ir, 5, 7);
          rs2 = bitcrop(ir, 5, 2);
          rs1 = 0;
        }
      }
      break;
    case 0b10001:
      if (bitcrop(ir, 3, 10) == 0b011 && bitcrop(ir, 2, 5) == 0b11) {
        // c.and.
        instruction = INST_AND;
        rd = rs1 = bitcrop(ir, 3, 7) + 8;
        rs2 = bitcrop(ir, 3, 2) + 8;
      } else if (bitcrop(ir, 3, 10) == 0b011 && bitcrop(ir, 2, 5) == 0b01) {
        instruction = INST_XOR;
        rd = rs1 = bitcrop(ir, 3, 7) + 8;
        rs2 = bitcrop(ir, 3, 2) + 8;
      } else if (bitcrop(ir, 3, 10) == 0b011 && bitcrop(ir, 2, 5) == 0b00) {
        instruction = INST_SUB;
        rd = rs1 = bitcrop(ir, 3, 7) + 8;
        rs2 = bitcrop(ir, 3, 2) + 8;
      } else if (bitcrop(ir, 3, 10) == 0b011 && bitcrop(ir, 2, 5) == 0b10) {
        instruction = INST_OR;
        rd = rs1 = bitcrop(ir, 3, 7) + 8;
        rs2 = bitcrop(ir, 3, 2) + 8;
      } else if (bitcrop(ir, 3, 10) == 0b111 && bitcrop(ir, 2, 5) == 0b01) {
        instruction = INST_ADDW;
        rd = rs1 = bitcrop(ir, 3, 7) + 8;
        rs2 = bitcrop(ir, 3, 2) + 8;
      } else if (bitcrop(ir, 3, 10) == 0b111 && bitcrop(ir, 2, 5) == 0b00) {
        instruction = INST_SUBW;
        rd = rs1 = bitcrop(ir, 3, 7) + 8;
        rs2 = bitcrop(ir, 3, 2) + 8;
      } else if (bitcrop(ir, 1, 11) == 0b0) {
        instruction = bitcrop(ir, 1, 10) == 1 ? INST_SRAI : INST_SRLI;
        rd = rs1 = bitcrop(ir, 3, 7) + 8;
        imm = (bitcrop(ir, 1, 12) << 5) + bitcrop(ir, 5, 2);
      } else if (bitcrop(ir, 1, 10) == 0b0) {
        instruction = INST_ANDI;
        rd = rs1 = bitcrop(ir, 3, 7) + 8;
        imm = (bitcrop(ir, 1, 12) << 5) + bitcrop(ir, 5, 2);
        imm = SignExtend(imm, 6);
      }
      break;
    case 0b10101:
      instruction = INST_JAL;
      rd = 0;
      imm = bitcrop(ir, 1, 12) << 11;
      imm |= bitcrop(ir, 1, 11) << 4;
      imm |= bitcrop(ir, 2, 9) << 8;
      imm |= bitcrop(ir, 1, 8) << 10;
      imm |= bitcrop(ir, 1, 7) << 6;
      imm |= bitcrop(ir, 1, 6) << 7;
      imm |= bitcrop(ir, 3, 3) << 1;
      imm |= bitcrop(ir, 1, 2) << 5;
      imm = SignExtend(imm, 12);
      break;
    case 0b11000:
      instruction = INST_SW;
      rs1 = bitcrop(ir, 3, 7) + 8;
      rs2 = bitcrop(ir, 3, 2) + 8;
      imm = bitcrop(ir, 3, 10) << 3;
      imm |= bitcrop(ir, 1, 6) << 2;
      imm |= bitcrop(ir, 1, 5) << 6;
      break;
    case 0b11100:
      instruction = INST_SD;
      rs1 = bitcrop(ir, 3, 7) + 8;
      rs2 = bitcrop(ir, 3, 2) + 8;
      imm = bitcrop(ir, 3, 10) << 3;
      imm |= bitcrop(ir, 2, 5) << 6;
      break;
    case 0b11001:
    case 0b11101:
      instruction = opcode == 0b11001 ? INST_BEQ : INST_BNE;
      rs1 = bitcrop(ir, 3, 7) + 8;
      rs2 = 0;
      imm = bitcrop(ir, 1, 12) << 8;
      imm |= bitcrop(ir, 2, 10) << 3;
      imm |= bitcrop(ir, 2, 5) << 6;
      imm |= bitcrop(ir, 2, 3) << 1;
      imm |= bitcrop(ir, 1, 2) << 5;
      imm = SignExtend(imm, 9);
      break;
    case 0b11010:
      instruction = INST_SW;
      rs1 = 2;
      rs2 = bitcrop(ir, 5, 2);
      imm = bitcrop(ir, 4, 9) << 2;
      imm |= bitcrop(ir, 2, 7) << 6;
      break;
    case 0b11110:
      instruction = INST_SD;
      rs1 = X2;
      rs2 = bitcrop(ir, 5, 2);
      imm = bitcrop(ir, 3, 10) << 3;
      imm |= bitcrop(ir, 3, 7) << 6;
      break;
    default:
      std::cerr << "Unsupported C Instruction." << std::endl;
      break;
  }
  *instruction_out = instruction;
  *rd_out = rd;
  *rs1_out = rs1;
  *rs2_out = rs2;
  *imm_out = imm;
}

}  // namespace RISCV_EMULATOR
