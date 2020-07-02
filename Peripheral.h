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
// Host Interface.
constexpr uint64_t kToHost0 = 0x80001000;
constexpr uint64_t kToHost1 = 0x80003000;
constexpr uint64_t kFromHost = 0x80001040;

// UART.
constexpr uint64_t kUartBase = 0x10000000;
constexpr uint64_t kUartSize = 6;

class Peripheral {
public:
  Peripheral(int mxl);

  void SetMemory(std::shared_ptr<MemoryWrapper> memory);

  void CheckPeripheralWrite(uint64_t address, int width, uint64_t data);

  void SetHostEmulationEnable(bool enable);
  void HostEmulation();
  bool PeripheralEmulation();

  bool GetHostEndFlag();

  uint64_t GetHostValue();

  bool GetHostErrorFlag();

  void UartEmulation();

private:
  std::shared_ptr<MemoryWrapper> memory_;
  int mxl_;
  bool host_emulation_ = false;
  int host_write_ = false;
  uint64_t host_value_ = 0;
  bool end_flag_ = false;
  bool error_flag_ = false;

  bool uart_enable_ = true;
  bool uart_write_ = false;
  uint8_t uart_write_value_ = 0;
};

} // namespace RISCV_EMULATOR

#endif //ASSEMBLER_TEST_PERIPHERAL_H
