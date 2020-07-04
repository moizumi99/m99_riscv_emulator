//
// Created by moiz on 7/2/20.
//

#include <iostream>
#include "PeripheralEmulator.h"
namespace RISCV_EMULATOR {

PeripheralEmulator::PeripheralEmulator(int mxl) : mxl_(mxl) {}

void PeripheralEmulator::SetMemory(std::shared_ptr<MemoryWrapper> memory) {
    memory_ = memory;
}

void PeripheralEmulator::SetHostEmulationEnable(bool enable) {
  host_emulation_enable_ = enable;
}

uint64_t PeripheralEmulator::GetHostValue() {
    return host_value_;
}

bool PeripheralEmulator::GetHostErrorFlag() {
  return error_flag_;
}

bool PeripheralEmulator::GetHostEndFlag() {
  return end_flag_;
}

void PeripheralEmulator::Initialize() {
  UartInit();
}

// reference: https://github.com/riscv/riscv-isa-sim/issues/364
void PeripheralEmulator::CheckDeviceWrite(uint64_t address, int width, uint64_t data) {
  // Check if the write is to host communication.
  if (mxl_ == 1) {
    host_write_ |= (address & 0xFFFFFFFF) == kToHost0 ? 1 : 0;
    host_write_ |= (address & 0xFFFFFFFF) == kToHost1 ? 2 : 0;
  } else {
    host_write_ |= address == kToHost0 ? 1 : 0;
    host_write_ |= address == kToHost1 ? 2 : 0;
  }
  // Check if it writes to UART addresses.
  if (kUartBase < address + width && address <= kUartBase) {
    uint64_t offset = kUartBase - address;
    uart_write_value_ = (data >> (offset * 8)) & 0xFF;
    uart_write_ = true;
  }
}

void PeripheralEmulator::Emulation() {
  if (host_emulation_enable_) {
    HostEmulation();
  }
  if (device_emulation_enable) {
    UartEmulation();
  }
}

// reference: https://github.com/riscv/riscv-isa-sim/issues/364
void PeripheralEmulator::HostEmulation() {
  if (host_write_ == 0) {
    return;
  }
  uint64_t payload;
  uint8_t device;
  uint32_t command;
  uint64_t value = 0;
  if ((host_write_ & 0b10) != 0) {
    payload = (mxl_ == 1) ? memory_->Read32(kToHost1) : memory_->Read64(kToHost1);
    host_value_ = payload >> 1;
    host_write_ = 0;
    end_flag_ = true;
    return;
  }

  end_flag_ = false;
  if (mxl_ == 1) {
    // This address should be physical.
    payload = memory_->Read32(kToHost0);
    device = 0;
    command = 0;
  } else {
    // This address should be physical.
    payload = memory_->Read64(kToHost0);
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
        host_value_ = value;
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
  host_write_ = 0;
  return;
}

void PeripheralEmulator::UartInit() {
  uint8_t isr = memory_->ReadByte(kUartBase + 5);
  isr |= (1 << 5);
  memory_->WriteByte(kUartBase + 5, isr);
}
void PeripheralEmulator::UartEmulation() {
  // UART Rx.
  if (uart_write_) {
    std::cout << static_cast<char>(uart_write_value_);
    uart_write_ = false;
  }
  // UART Tx.
//  std::string input_string;
//  std::cin >> input_string;
//  if (!input_string.empty()) {
//    for (auto s : input_string) {
//      uart_queue.push(static_cast<uint8_t>(s));
//    }
//  }

}

void PeripheralEmulator::TimerTick() {
  elapsed_cycles++;
  uint64_t next_cycle = memory_->Read64(kTimerBase);
  if (elapsed_cycles == next_cycle) {
    timer_interrupt_ = true;
  }
}

uint64_t PeripheralEmulator::GetTimerInterrupt() {
  return timer_interrupt_;
}

void PeripheralEmulator::ClearTimerInterrupt() {
  timer_interrupt_ = false;
}

} // namespace RISCV_EMULATOR