//
// Created by moiz on 7/2/20.
//

#include <iostream>
#include "Peripheral.h"
namespace RISCV_EMULATOR {

Peripheral::Peripheral(int mxl) : mxl_(mxl) {}

void Peripheral::SetMemory(std::shared_ptr<MemoryWrapper> memory) {
    memory_ = memory;
}

uint64_t Peripheral::GetHostValue() {
    return host_value_;
}

int Peripheral::GetHostWrite() {
  return host_write_;
}

bool Peripheral::GetHostErrorFlag() {
  return error_flag_;
}

// reference: https://github.com/riscv/riscv-isa-sim/issues/364
void Peripheral::CheckHostWrite(uint64_t address) {
  // Check if the write is to host communication.
  if (mxl_ == 1) {
    host_write_ |= (address & 0xFFFFFFFF) == kToHost0 ? 1 : 0;
    host_write_ |= (address & 0xFFFFFFFF) == kToHost1 ? 2 : 0;
  } else {
    host_write_ |= address == kToHost0 ? 1 : 0;
    host_write_ |= address == kToHost1 ? 2 : 0;
  }
}

// reference: https://github.com/riscv/riscv-isa-sim/issues/364
bool Peripheral::HostEmulation() {
  uint64_t payload;
  uint8_t device;
  uint32_t command;
  uint64_t value = 0;
  if ((host_write_ & 0b10) != 0) {
    payload = (mxl_ == 1) ? memory_->Read32(kToHost1) : memory_->Read64(kToHost1);
    host_value_ = payload >> 1;
    host_write_ = 0;
    return true;
  }

  bool end_flag = false;
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
        end_flag = true;
      }
    } else {
      std::cerr << "Unsupported Host command " << command << " for Device 0"
                << std::endl;
      error_flag_ = true;
      end_flag = true;
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
  return end_flag;
}

} // namespace RISCV_EMULATOR