//
// Created by moiz on 7/2/20.

//
#ifndef ASSEMBLER_TEST_PERIPHERAL_H
#define ASSEMBLER_TEST_PERIPHERAL_H

#include <cstdint>
#include <memory>
#include "memory_wrapper.h"

namespace RISCV_EMULATOR {

// Memory Mapped Register Addresses.
constexpr uint64_t kToHost0 = 0x80001000;
constexpr uint64_t kToHost1 = 0x80003000;
constexpr uint64_t kFromHost = 0x80001040;

class Peripheral {
public:
  Peripheral(int mxl);

  void SetMemory(std::shared_ptr<MemoryWrapper> memory);

  void CheckHostWrite(uint64_t address);

  bool HostEmulation();

  int GetHostWrite();

  uint64_t GetHostValue();

  bool GetHostErrorFlag();

private:
  std::shared_ptr<MemoryWrapper> memory_;
  int mxl_;
  int host_write_ = false;
  uint64_t host_value_ = 0;
  bool error_flag_ = false;

};

} // namespace RISCV_EMULATOR

#endif //ASSEMBLER_TEST_PERIPHERAL_H
