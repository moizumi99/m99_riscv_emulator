//
// Created by moiz on 7/2/20.

//
#ifndef ASSEMBLER_TEST_PERIPHERALEMULATOR_H
#define ASSEMBLER_TEST_PERIPHERALEMULATOR_H

#include <cstdint>
#include <memory>
#include <queue>
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

// Timer.
constexpr uint64_t kTimerBase = 0x2000000;

class PeripheralEmulator {
public:
  PeripheralEmulator(int mxl);

  void SetMemory(std::shared_ptr<MemoryWrapper> memory);
  void Emulation();

  void Initialize();
  void CheckDeviceWrite(uint64_t address, int width, uint64_t data);

  // Host Emulation.
  void SetHostEmulationEnable(bool enable);
  void HostEmulation();

  bool GetHostEndFlag();

  uint64_t GetHostValue();

  bool GetHostErrorFlag();

  // Clock Tick.
  void TimerTick();
  uint64_t GetTimerInterrupt();
  void ClearTimerInterrupt();

  // Device Emulation.
  void SetDeviceEmulationEnable(bool enable) {
    device_emulation_enable = enable;
  }
  void UartInit();
  void UartEmulation();

private:
  std::shared_ptr<MemoryWrapper> memory_;
  int mxl_;
  bool host_emulation_enable_ = false;
  int host_write_ = false;
  uint64_t host_value_ = 0;
  bool end_flag_ = false;
  bool error_flag_ = false;

  // UART emulation enable.
  bool device_emulation_enable = false;
  bool uart_write_ = false;
  bool uart_read_ = false;
  uint8_t uart_write_value_ = 0;
  std::queue<uint8_t> uart_queue;

  // Timer.
  uint64_t elapsed_cycles = 0;
  bool timer_interrupt_ = false;
};

} // namespace RISCV_EMULATOR

#endif //ASSEMBLER_TEST_PERIPHERALEMULATOR_H
