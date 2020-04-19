#include "RISCV_cpu.h"
#include "bit_tools.h"
#include "instruction_encdec.h"
#include "memory_wrapper.h"
#include "system_call_emulator.h"
#include "pte.h"
#include <iostream>
#include <tuple>
#include <stdint.h>
#include <cassert>

RiscvCpu::RiscvCpu(bool en64bit) : xlen(en64bit ? 64 : 32) {
  privilege_ = PrivilegeMode::MACHINE_MODE;
  mstatus_ = 0;
  for (int i = 0; i < kRegNum; i++) {
    reg_[i] = 0;
  }
  csrs_.resize(kCsrSize, 0);
}

RiscvCpu::RiscvCpu() : RiscvCpu(false) {}


constexpr int kPageSize = 1 << 12; // PAGESIZE is 2^12.
constexpr int kMmuLevels = 2;

uint64_t RiscvCpu::VirtualToPhysical(uint64_t virtual_address, bool write_access) {
  if (privilege_ != PrivilegeMode::USER_MODE && privilege_ != PrivilegeMode::SUPERVISOR_MODE) {
    return virtual_address;
  }
  if (xlen == 32) {
    return VirtualToPhysical32(virtual_address, write_access);
  } else {
    // if (xlen == 64) {
    return VirtualToPhysical64(virtual_address, write_access);
  }
}

uint64_t RiscvCpu::VirtualToPhysical64(uint64_t virtual_address, bool write_access) {
  // TODO: Implement v48 MMU emulation.
  constexpr int kPteSize = 8;
  uint64_t physical_address = virtual_address;
  MemoryWrapper &mem = *memory_;
  uint64_t satp = csrs_[SATP];
  uint8_t mode = bitcrop(satp, 4, 60);
  if (mode == 0) {
    return physical_address;
  } else if (mode != 8) {
    std::cerr << "Unsupported virtual address translation mode (" << mode << ")" << std::endl;
    page_fault_ = true;
    faulting_address = virtual_address;
    return physical_address;
  }
  // uint16_t asid = bitcrop(sptbr, 9, 22);
  uint64_t ppn = bitcrop(satp, 44, 0);
  uint64_t vpn[3];
  vpn[2] = bitcrop(virtual_address, 9, 30);
  vpn[1] = bitcrop(virtual_address, 9, 21);
  vpn[0] = bitcrop(virtual_address, 9, 12);
  uint16_t offset = bitcrop(virtual_address, 12, 0);
  Pte64 pte;
  int level;
  uint64_t pte_address;
  constexpr int k64BitMmuLevels = 3;
  for (level = k64BitMmuLevels - 1; level >= 0; --level) {
    pte_address = ppn * kPageSize + vpn[level] * kPteSize;
    uint64_t pte_value = mem.Read64(pte_address);
    pte = pte_value;
    if (!pte.IsValid()) {
      std::cerr << "PTE not valid." << std::endl;
      std::cerr << "PTE = " << std::hex << pte.GetValue() << std::endl;
      std::cerr << "virtual_address = " << virtual_address << std::endl;
      std::cerr << "PTE entry address = " << pte_address << std::endl;
      page_fault_ = true;
      faulting_address = virtual_address;
      return physical_address;
    }
    if (pte.IsLeaf()) {
      break;
    }
    if (level == 0) {
      std::cerr << "Non-leaf block in level 0." << std::endl;
      page_fault_ = true;
      faulting_address = virtual_address;
      return physical_address;
    }
    ppn = pte.GetPpn();
  }
  if ((level > 0 && pte.GetPpn0() != 0) || (level > 1 && pte.GetPpn1() != 0)) {
    // Misaligned superpage.
    std::cerr << "Misaligned super page." << std::endl;
    page_fault_ = true;
    faulting_address = virtual_address;
    return physical_address;
  }
  // Access and Dirty bit process;
  pte.SetA(1);
  if (write_access) {
    pte.SetD(1);
  }
  mem.Write64(pte_address, pte.GetValue());
  // TODO: Add PMP check. (Page 70 of RISC-V Privileged Architectures Manual Vol. II.)
  uint64_t ppn2 = pte.GetPpn2();
  uint64_t ppn1 = (level > 1) ? vpn[1] : pte.GetPpn1();
  uint32_t ppn0 = (level > 0) ? vpn[0] : pte.GetPpn0();
  physical_address = (ppn2 << 30) | (ppn1 << 21) | (ppn0 << 12) | offset;

  uint64_t physical_address_64bit = static_cast<uint64_t >(physical_address & GenMask<int64_t>(56, 0));
  return physical_address_64bit;
}

uint64_t RiscvCpu::VirtualToPhysical32(uint64_t virtual_address, bool write_access) {
  constexpr int kPteSize = 4;
  uint64_t physical_address = virtual_address;
  MemoryWrapper &mem = *memory_;
  uint32_t satp = csrs_[SATP];
  uint8_t mode = bitcrop(satp, 1, 31);
  if (mode == 0) {
    return physical_address;
  }
  // uint16_t asid = bitcrop(sptbr, 9, 22);
  uint32_t ppn = bitcrop(satp, 22, 0);
  uint16_t vpn1 = bitcrop(virtual_address, 10, 22);
  uint16_t vpn0 = bitcrop(virtual_address, 10, 12);
  uint16_t offset = bitcrop(virtual_address, 12, 0);
  Pte32 pte;
  int level;
  uint32_t vpn = vpn1;
  uint32_t pte_address;
  for (level = kMmuLevels - 1; level >= 0; --level) {
    pte_address = ppn * kPageSize + vpn * kPteSize;
    uint32_t pte_value = mem.Read32(pte_address);
    pte = pte_value;
    if (!pte.IsValid()) {
      // TODO: Do page-fault exception.
      std::cerr << "PTE not valid." << std::endl;
      std::cerr << "PTE = " << std::hex << pte.GetValue() << std::endl;
      std::cerr << "virtual_address = " << virtual_address << std::endl;
      std::cerr << "PTE entry address = " << pte_address << std::endl;
      page_fault_ = true;
      faulting_address = virtual_address;
      return physical_address;
    }
    if (pte.IsLeaf()) {
      break;
    }
    if (level == 0) {
      std::cerr << "Non-leaf block in level 0." << std::endl;
      page_fault_ = true;
      faulting_address = virtual_address;
      return physical_address;
    }
    ppn = pte.GetPpn();
    vpn = vpn0;
  }
  if (level > 0 && pte.GetPpn0() != 0) {
    // Misaligned superpage.
    // TODO: Do page-fault exception.
    std::cerr << "Misaligned super page." << std::endl;
    page_fault_ = true;
    faulting_address = virtual_address;
    return physical_address;
  }
  // Access and Dirty bit process;
  pte.SetA(1);
  if (write_access) {
    pte.SetD(1);
  }
  mem.Write32(pte_address, pte.GetValue());
  // TODO: Add PMP check. (Page 70 of RISC-V Privileged Architectures Manual Vol. II.)
  uint64_t ppn1 = pte.GetPpn1();
  uint32_t ppn0 = (level == 1) ? vpn0 : pte.GetPpn0();
  physical_address = (ppn1 << 22) | (ppn0 << 12) | offset;

  uint64_t physical_address_64bit = static_cast<uint64_t >(physical_address & 0xFFFFFFFF);
  return physical_address_64bit;
}

// A helper function to record shift sign error.
bool RiscvCpu::CheckShiftSign(uint8_t shamt, uint8_t instruction, const std::string &message_str) {
  if (xlen == 32 || instruction == INST_SLLIW || instruction == INST_SRAIW || instruction == INST_SRLIW) {
    if (shamt >> 5) {
      std::cerr << message_str << " Shift value (shamt) error. shamt = " << static_cast<int>(shamt) << std::endl;
      return true;
    }
  }
  return false;
}

void RiscvCpu::SetRegister(uint32_t num, uint64_t value) { reg_[num] = value; }

void RiscvCpu::SetMemory(std::shared_ptr<MemoryWrapper> memory) {
  this->memory_ = memory;
}

uint32_t RiscvCpu::LoadCmd(uint64_t pc) {
  auto &mem = *memory_;
  uint64_t physical_address = VirtualToPhysical(pc);
  return mem.Read32(physical_address);
}

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
  uint64_t result = (width == 32) ? mem.Read32(physical_address) : mem.Read64(physical_address);
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

void RiscvCpu::Exception(int cause) {
  // Currently supported exceptions: page fault (12, 13, 15) and ecall (8, 9, 11).
  assert(
    cause == INSTRUCTION_PAGE_FAULT || cause == LOAD_PAGE_FAULT || cause == STORE_PAGE_FAULT || cause == ECALL_UMODE ||
    cause == ECALL_SMODE || cause == ECALL_MMODE);
  // Page fault specific processing.
  if (cause == INSTRUCTION_PAGE_FAULT || cause == LOAD_PAGE_FAULT || cause == STORE_PAGE_FAULT) {
    if (prev_page_fault_ && prev_faulting_address_ == faulting_address) {
      std::cerr << "This is a page fault right after another fault at the same address. Exit." << std::endl;
      error_flag_ = true;
    }
    prev_page_fault_ = page_fault_;
    prev_faulting_address_ = faulting_address;
    page_fault_ = false;
  }

  // TODO: Fix trap level. Current implementation always treat them as machine-exception.
  // Check delegation status.
  if (((csrs_[MEDELEG] >> cause) & 1) && ((csrs_[SEDELEG] >> cause) & 1)) {
    // Delegate to U-Mode.
    csrs_[UCAUSE] = cause;
    csrs_[UEPC] = pc_;
    uint64_t tvec = csrs_[UTVAL];
    uint8_t mode = tvec & 0b11;
    if (mode == 0) {
      next_pc_ = tvec & ~0b11;
    } else {
      next_pc_ = (tvec & ~0b11) + 4 * (csrs_[UCAUSE] & GenMask<uint64_t>(xlen - 1, 0));

    }
    csrs_[UTVAL] = faulting_address;
    // Copy old UIE (#0) to UPIE ($4). Then, disable UIE (#0)
    const uint64_t uie = bitcrop(mstatus_, 1, 0);
    mstatus_ = bitset(mstatus_, 1, 4, uie);
    uint64_t new_uie = 0;
    mstatus_ = bitset(mstatus_, 1, 0, new_uie);
    privilege_ = PrivilegeMode::USER_MODE;
  } else if ((csrs_[MEDELEG] >> cause) & 1) {
    // Delegated to S-Mode.
    csrs_[SCAUSE] = cause;
    csrs_[SEPC] = pc_;
    uint64_t tvec = csrs_[STVEC];
    uint8_t mode = tvec & 0b11;
    if (mode == 0) {
      next_pc_ = tvec & ~0b11;
    } else {
      next_pc_ = (tvec & ~0b11) + 4 * (csrs_[SCAUSE] & GenMask<uint64_t>(xlen - 1, 0));
    }
    csrs_[STVAL] = faulting_address;

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
    if (mode == 0) {
      next_pc_ = tvec & ~0b11;
    } else {
      next_pc_ = (tvec & ~0b11) + 4 * (csrs_[MCAUSE] & GenMask<uint64_t>(xlen - 1, 0));
    }
    // Store the faulting address to MTVAL.
    csrs_[MTVAL] = faulting_address;

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
  const uint64_t sstatus_mask = (1 << (xlen - 1) & 0b11011110000100110011);
  const uint64_t ustatus_mask = (1 << (xlen - 1) & 0b11011110000000010001);
  csrs_[USTATUS] = mstatus_ & ustatus_mask;
  csrs_[SSTATUS] = mstatus_ & sstatus_mask;
  csrs_[MSTATUS] = mstatus_;
}

uint64_t kUpper32bitMask = 0xFFFFFFFF00000000;

uint64_t RiscvCpu::Sext32bit(uint64_t data32bit) {
  if ((data32bit >> 31) & 1) {
    return data32bit | kUpper32bitMask;
  } else {
    return data32bit & (~kUpper32bitMask);
  }
}

int RiscvCpu::RunCpu(uint64_t start_pc, bool verbose) {
  error_flag_ = false;
  end_flag_ = false;

  next_pc_ = start_pc;
  do {
    pc_ = next_pc_;
    uint64_t ir;

    ir = LoadCmd(pc_);
    if (page_fault_) {
      Exception(ExceptionCode::INSTRUCTION_PAGE_FAULT);
      continue;
    }
    if (verbose) {
      printf("PC: %016lx, cmd: %016lx, privilege: %d\n", pc_, ir, static_cast<int>(privilege_)
      );
      std::cout << "           X1/RA            X2/SP            X3/GP            X4/TP            "
                   "X5/T0            X6/T1            X7/T2            X8/S0/FP         "
                   "X9/S1            X10/A0           X11/A1           X12/A2           "
                   "X13/A3           X14/A4           X15/A5           X16/A6 " << std::endl;
      printf("%016lx %016lx %016lx %016lx %016lx %016lx %016lx %016lx "
             "%016lx %016lx %016lx %016lx %016lx %016lx %016lx %016lx\n",
             reg_[1], reg_[2], reg_[3], reg_[4], reg_[5], reg_[6], reg_[7], reg_[8],
             reg_[9], reg_[10], reg_[11], reg_[12], reg_[13], reg_[14], reg_[15], reg_[16]
      );
      std::cout << "          X17/A7           X18/S2           X19/S3           X20/S4           "
                   "X21/S5           X22/S6           X23/S7           X24/S8           "
                   "X25/S9           X26/S10          X27/S11          X28/T3           "
                   "X29/T4           X30/T5           X31/T6" << std::endl;
      printf("%016lx %016lx %016lx %016lx %016lx %016lx %016lx %016lx "
             "%016lx %016lx %016lx %016lx %016lx %016lx %016lx\n",
             reg_[17], reg_[18], reg_[19], reg_[20], reg_[21], reg_[22], reg_[23], reg_[24],
             reg_[25], reg_[26], reg_[27], reg_[28], reg_[29], reg_[30], reg_[31]
      );
    }

    // Change this line when C is supported.
    next_pc_ = pc_ + 4;

    // Decode. Mimick the HW behavior. (In HW, decode is in parallel.)
    uint8_t instruction = GetCode(ir);
    uint8_t rd = GetRd(ir);
    uint8_t rs1 = GetRs1(ir);
    uint8_t rs2 = GetRs2(ir);
    int16_t imm12 = GetImm12(ir);
    int16_t csr = GetCsr(ir);
    uint8_t shamt = GetShamt(ir);
    int16_t imm13 = GetImm13(ir);
    int32_t imm21 = GetImm21(ir);
    int16_t imm12_stype = GetStypeImm12(ir);
    int32_t imm20 = GetImm20(ir);
    uint32_t address;

    uint64_t load_data;
    uint32_t shift_mask = xlen == 64 ? 0b0111111 : 0b0011111;
    switch (instruction) {
      uint64_t t;
      uint64_t temp64;
      case INST_ADD:
        temp64 = reg_[rs1] + reg_[rs2];
        if (xlen == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_ADDW:
        assert(xlen == 64);
        temp64 = (reg_[rs1] + reg_[rs2]) & 0xFFFFFFFF;
        temp64 = Sext32bit(temp64);
        reg_[rd] = temp64;
        break;
      case INST_AND:
        temp64 = reg_[rs1] & reg_[rs2];
        if (xlen == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_SUB:
        temp64 = reg_[rs1] - reg_[rs2];
        if (xlen == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_SUBW:
        temp64 = (reg_[rs1] - reg_[rs2]) & 0xFFFFFFFF;
        temp64 = Sext32bit(temp64);
        reg_[rd] = temp64;
        break;
      case INST_OR:
        temp64 = reg_[rs1] | reg_[rs2];
        if (xlen == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_XOR:
        temp64 = reg_[rs1] ^ reg_[rs2];
        if (xlen == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_SLL:
        temp64 = reg_[rs1] << (reg_[rs2] & shift_mask);
        if (xlen == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_SLLW:
        temp64 = (reg_[rs1] << (reg_[rs2] & 0b011111)) & 0x0FFFFFFFF;
        temp64 = Sext32bit(temp64);
        reg_[rd] = temp64;
        break;
      case INST_SRL:
        temp64 = reg_[rs1];
        if (xlen == 32) {
          temp64 &= ~kUpper32bitMask;
        }
        temp64 = temp64 >> (reg_[rs2] & shift_mask);
        if (xlen == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_SRLW:
        temp64 = (reg_[rs1] & 0xFFFFFFFF) >> (reg_[rs2] & 0b011111);
        temp64 = Sext32bit(temp64);
        reg_[rd] = temp64;
        break;
      case INST_SRA:
        reg_[rd] = static_cast<int64_t>(reg_[rs1]) >> (reg_[rs2] & shift_mask);
        break;
      case INST_SRAW:
        reg_[rd] = static_cast<int32_t>(reg_[rs1]) >> (reg_[rs2] & 0b011111);
        break;
      case INST_SLT:
        reg_[rd] = (static_cast<int64_t>(reg_[rs1]) < static_cast<int64_t>(reg_[rs2])) ? 1 : 0;
        break;
      case INST_SLTU:
        reg_[rd] = (reg_[rs1] < reg_[rs2]) ? 1 : 0;
        break;
      case INST_ADDI:
        temp64 = reg_[rs1] + imm12;
        if (xlen == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_ADDIW:
        // Add instruction set check.
        assert(xlen == 64);
        temp64 = (reg_[rs1] + imm12) & 0xFFFFFFFF;
        temp64 = Sext32bit(temp64);
        reg_[rd] = temp64;
        break;
      case INST_ANDI:
        temp64 = reg_[rs1] & imm12;
        if (xlen == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_ORI:
        temp64 = reg_[rs1] | imm12;
        if (xlen == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_XORI:
        temp64 = reg_[rs1] ^ imm12;
        if (xlen == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_SLLI:
        temp64 = reg_[rs1] << shamt;
        temp64 = Sext32bit(temp64);
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
        if (xlen == 32) {
          temp64 &= ~kUpper32bitMask;
        }
        temp64 = temp64 >> shamt;
        if (xlen == 32) {
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
      case INST_SLTI:
        reg_[rd] = static_cast<int64_t>(reg_[rs1]) < imm12 ? 1 : 0;
        break;
      case INST_SLTIU:
        reg_[rd] = reg_[rs1] < static_cast<uint64_t>(imm12) ? 1 : 0;
        break;
      case INST_BEQ:
        if (reg_[rs1] == reg_[rs2]) {
          next_pc_ = pc_ + SignExtend(imm13, 13);
        }
        break;
      case INST_BGE:
        if (static_cast<int64_t>(reg_[rs1]) >= static_cast<int64_t>(reg_[rs2])) {
          next_pc_ = pc_ + imm13;
        }
        break;
      case INST_BGEU:
        if (reg_[rs1] >= reg_[rs2]) {
          next_pc_ = pc_ + imm13;
        }
        break;
      case INST_BLT:
        if (static_cast<int64_t>(reg_[rs1]) < static_cast<int64_t>(reg_[rs2])) {
          next_pc_ = pc_ + imm13;
        }
        break;
      case INST_BLTU:
        if (reg_[rs1] < reg_[rs2]) {
          next_pc_ = pc_ + imm13;
        }
        break;
      case INST_BNE:
        if (reg_[rs1] != reg_[rs2]) {
          next_pc_ = pc_ + imm13;
        }
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
        address = VirtualToPhysical(reg_[rs1] + imm12);
        if (page_fault_) {
          break;
        }
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
        if (page_fault_) {
          Exception(ExceptionCode::LOAD_PAGE_FAULT);
        }
        break;
      case INST_SB:
      case INST_SH:
      case INST_SW:
      case INST_SD:
        address = VirtualToPhysical(reg_[rs1] + imm12_stype, true);
        if (page_fault_) {
          break;
        }
        if (instruction == INST_SB) {
          StoreWd(address, reg_[rs2], 8);
        } else if (instruction == INST_SH) {
          StoreWd(address, reg_[rs2], 16);
        } else if (instruction == INST_SW) {
          StoreWd(address, reg_[rs2], 32);
        } else { // if instruction === INST_SD.
          StoreWd(address, reg_[rs2], 64);
        }
        if (page_fault_) {
          Exception(ExceptionCode::STORE_PAGE_FAULT);
        }
        break;
      case INST_LUI:
        temp64 = imm20 << 12;
        if (xlen == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_AUIPC:
        temp64 = pc_ + (imm20 << 12);
        if (xlen == 32) {
          temp64 = Sext32bit(temp64);
        }
        reg_[rd] = temp64;
        break;
      case INST_SYSTEM:
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
        } else if (imm12 >> 5 == 0b0001001 & rd == 0b00000) {
          // sfence.vma.
          // TODO: Implement this function.
          ;
        } else {
          // not defined.
          std::cerr << "Undefined System instruction." << std::endl;
          error_flag_ = true;
        }
        break;
      case INST_CSRRC:
        t = csrs_[csr];
        csrs_[csr] &= ~reg_[rs1];
        reg_[rd] = t;
        UpdateMstatus(csr);
        break;
      case INST_CSRRCI:
        t = csrs_[csr];
        csrs_[csr] &= ~rs1;
        reg_[rd] = t;
        UpdateMstatus(csr);
        break;
      case INST_CSRRS:
        t = csrs_[csr];
        csrs_[csr] |= reg_[rs1];
        reg_[rd] = t;
        UpdateMstatus(csr);
        break;
      case INST_CSRRSI:
        t = csrs_[csr];
        csrs_[csr] |= rs1;
        reg_[rd] = t;
        UpdateMstatus(csr);
        break;
      case INST_CSRRW:
        t = csrs_[csr];
        csrs_[csr] = reg_[rs1];
        reg_[rd] = t;
        UpdateMstatus(csr);
        break;
      case INST_CSRRWI:
        t = csrs_[csr];
        csrs_[csr] = rs1;
        reg_[rd] = t;
        UpdateMstatus(csr);
        break;
      case INST_FENCE:
      case INST_FENCEI:
        // Do nothing for now.
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
  } while (!error_flag_ && !end_flag_);

  if (error_flag_ && verbose) {
    DumpCpuStatus();
  }
  return error_flag_;
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
  Exception(cause);
}

void RiscvCpu::UpdateMstatus(const int16_t csr) {
  if (csr != USTATUS && csr != SSTATUS && csr != MSTATUS) {
    return;
  }
  const uint64_t sstatus_mask = (1 << (xlen - 1) & 0b11011110000100110011);
  const uint64_t ustatus_mask = (1 << (xlen - 1) & 0b11011110000000010001);
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
  csrs_[USTATUS] = mstatus_ & ustatus_mask;
  csrs_[SSTATUS] = mstatus_ & sstatus_mask;
  csrs_[MSTATUS] = mstatus_;
}

void RiscvCpu::DumpCpuStatus() {
  std::cout << "CPU Privilege Mode = " << static_cast<int>(privilege_) << std::endl;
  std::cout << "CSR[MSTATUS] = " << std::hex << csrs_[MSTATUS] << std::endl;
  std::cout << "PC = " << std::hex << pc_ << std::endl;
}

PrivilegeMode RiscvCpu::ToPrivilegeMode(int value) {
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
  privilege_ = ToPrivilegeMode(mpp);
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
  privilege_ = ToPrivilegeMode(spp);
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
      if (funct3 == FUNC3_ADDSUB) {
        instruction = (funct7 == FUNC_NORM) ? INST_ADD : INST_SUB;
      } else if (funct3 == FUNC3_AND) {
        instruction = INST_AND;
      } else if (funct3 == FUNC3_OR) {
        instruction = INST_OR;
      } else if (funct3 == FUNC3_XOR) {
        instruction = INST_XOR;
      } else if (funct3 == FUNC3_SR) {
        if (funct7 == 0b0000000) {
          instruction = INST_SRL;
        } else if (funct7 == 0b0100000) {
          instruction = INST_SRA;
        }
      } else if (funct3 == FUNC3_SL) {
        instruction = INST_SLL;
      } else if (funct3 == FUNC3_SLT) {
        instruction = INST_SLT;
      } else if (funct3 == FUNC3_SLTU) {
        instruction = INST_SLTU;
      }
      break;
    case OPCODE_ARITHLOG_64:
      if (funct3 == FUNC3_ADDSUB) {
        instruction = (funct7 == FUNC_NORM) ? INST_ADDW : INST_SUBW;
      } else if (funct3 == FUNC3_SL) {
        instruction = INST_SLLW;
      } else if (funct3 == FUNC3_SR) {
        if (funct7 == 0b000000) {
          instruction = INST_SRLW;
        } else if (funct7 == 0b0100000) {
          instruction = INST_SRAW;
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


