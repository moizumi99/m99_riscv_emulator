//
// Created by moiz on 5/10/20.
//

#include <iostream>
#include "Mmu.h"
#include "bit_tools.h"
#include "pte.h"

Mmu::Mmu(std::shared_ptr<MemoryWrapper> memory, const int mxl) : memory_(memory), mxl_(mxl) {
  page_fault_ = false;
  faulting_address_ = 0;
}

void Mmu::SetPrivilege(const PrivilegeMode privilege) {
  privilege_ = privilege;
}

uint64_t Mmu::VirtualToPhysical(uint64_t virtual_address, uint64_t satp, bool write_access) {
  if (privilege_ != PrivilegeMode::USER_MODE &&
      privilege_ != PrivilegeMode::SUPERVISOR_MODE) {
    return virtual_address;
  }
  if (mxl_ == 1) {
    return VirtualToPhysical32(virtual_address, satp, write_access);
  } else {
    // if (xlen == 64) {
    return VirtualToPhysical64(virtual_address, satp, write_access);
  }
}

uint64_t Mmu::VirtualToPhysical64(uint64_t virtual_address, uint64_t satp, bool write_access) {
  // TODO: Implement v48 MMU emulation.
  constexpr int kPteSize = 8;
  uint64_t physical_address = virtual_address;
  MemoryWrapper &mem = *memory_;
//  uint64_t satp = csrs_[SATP];
  uint8_t mode = bitcrop(satp, 4, 60);
  if (mode == 0) {
    return physical_address;
  } else if (mode != 8) {
    std::cerr << "Unsupported virtual address translation mode (" << mode << ")"
              << std::endl;
    page_fault_ = true;
    faulting_address_ = virtual_address;
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
      faulting_address_ = virtual_address;
      return physical_address;
    }
    if (pte.IsLeaf()) {
      break;
    }
    if (level == 0) {
      std::cerr << "Non-leaf block in level 0." << std::endl;
      page_fault_ = true;
      faulting_address_ = virtual_address;
      return physical_address;
    }
    ppn = pte.GetPpn();
  }
  if ((level > 0 && pte.GetPpn0() != 0) || (level > 1 && pte.GetPpn1() != 0)) {
    // Misaligned superpage.
    std::cerr << "Misaligned super page." << std::endl;
    page_fault_ = true;
    faulting_address_ = virtual_address;
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

  uint64_t physical_address_64bit = static_cast<uint64_t >(physical_address &
                                                           GenMask<int64_t>(56,
                                                                            0));
  return physical_address_64bit;
}

uint64_t Mmu::VirtualToPhysical32(uint64_t virtual_address, uint64_t satp, bool write_access) {
  constexpr int kPteSize = 4;
  uint64_t physical_address = virtual_address;
  MemoryWrapper &mem = *memory_;
  // uint32_t satp = csrs_[SATP];
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
      faulting_address_ = virtual_address;
      return physical_address;
    }
    if (pte.IsLeaf()) {
      break;
    }
    if (level == 0) {
      std::cerr << "Non-leaf block in level 0." << std::endl;
      page_fault_ = true;
      faulting_address_ = virtual_address;
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
    faulting_address_ = virtual_address;
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

  uint64_t physical_address_64bit = static_cast<uint64_t >(physical_address &
                                                           0xFFFFFFFF);
  return physical_address_64bit;
}

