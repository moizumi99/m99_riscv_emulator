//
// Created by moiz on 5/10/20.
//

#ifndef ASSEMBLER_TEST_MMU_H
#define ASSEMBLER_TEST_MMU_H

#include <memory>
#include "riscv_cpu_common.h"
#include "memory_wrapper.h"

namespace RISCV_EMULATOR  {

class Mmu {
public:
  Mmu(std::shared_ptr<MemoryWrapper> memory, const int mxl);
  void SetPrivilege(const PrivilegeMode privilege);
  uint64_t VirtualToPhysical(uint64_t virtual_address, uint64_t satp,
                             bool write_access = false);
  bool GetPageFault() {return page_fault_;}
  uint64_t GetFaultingAddress() {return faulting_address_;}
private:
  uint64_t VirtualToPhysical32(uint64_t virtual_address, uint64_t satp,
                               bool write_access = false);

  uint64_t VirtualToPhysical64(uint64_t virtual_address, uint64_t satp,
                               bool write_access = false);

  std::shared_ptr<MemoryWrapper> memory_;
  int mxl_;
  bool page_fault_;
  uint64_t faulting_address_;
  PrivilegeMode privilege_;
  static constexpr int kPageSize = 1 << 12; // PAGESIZE is 2^12.
  static constexpr int kMmuLevels = 2;
};

} // namespace RISCV_EMULATOR

#endif //ASSEMBLER_TEST_MMU_H
