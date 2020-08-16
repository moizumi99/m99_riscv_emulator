//
// Created by moiz on 7/2/20.
//

#include "PeripheralEmulator.h"
#include <cassert>
#include <iostream>
namespace RISCV_EMULATOR {

PeripheralEmulator::PeripheralEmulator(int mxl) : mxl_(mxl) {}

void PeripheralEmulator::SetMemory(std::shared_ptr<MemoryWrapper> memory) { memory_ = memory; }

void PeripheralEmulator::SetDiskImage(std::shared_ptr<std::vector<uint8_t>> disk_image) {disk_image_ = disk_image; };

void PeripheralEmulator::SetHostEmulationEnable(bool enable) { host_emulation_enable_ = enable; }

uint64_t PeripheralEmulator::GetHostValue() { return host_value_; }

bool PeripheralEmulator::GetHostErrorFlag() { return error_flag_; }

bool PeripheralEmulator::GetHostEndFlag() { return end_flag_; }

void PeripheralEmulator::Initialize() {
  UartInit();
  VirtioInit();
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
  if (kVirtioBase < address + width && address <= kVirtioEnd) {
    virtio_address_ = address;
    virtio_data_ = data;
    virtio_width_ = width;
    virtio_write_ = true;
  }
  if (kTimerCmp < address + width && address < kTimerCmp + 8) {
    timercmp_update_ = true;
  }
}

void PeripheralEmulator::MemoryMappedValueUpdate() {
  memory_->Write64(kTimerMtime, elapsed_cycles_);
}

void PeripheralEmulator::Emulation() {
  if (host_emulation_enable_) {
    HostEmulation();
  }
  if (device_emulation_enable) {
    UartEmulation();
    VirtioEmulation();
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
      std::cerr << "Unsupported Host command " << command << " for Device 0" << std::endl;
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
      std::cerr << "Unsupported host command " << command << " for Device 1" << std::endl;
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
    std::cout << static_cast<char>(uart_write_value_) << std::flush;
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
  ++elapsed_cycles_;
  if (timercmp_update_) {
    next_cycle_ = memory_->Read64(kTimerCmp);
    timercmp_update_ = false;
  }
  if (elapsed_cycles_ == next_cycle_) {
    timer_interrupt_ = true;
  }
}

uint64_t PeripheralEmulator::GetTimerInterrupt() { return timer_interrupt_; }

void PeripheralEmulator::ClearTimerInterrupt() { timer_interrupt_ = false; }

void PeripheralEmulator::VirtioInit() {
  assert(memory_);
  memory_->Write32(kVirtioBase + 0x00, 0x74726976);
  memory_->Write32(kVirtioBase + 0x4, 1);
  memory_->Write32(kVirtioBase + 0x8, 2);
  memory_->Write32(kVirtioBase + 0xc, 0x554d4551);
  memory_->Write32(kVirtioBase + 0x34, 8);
}

void PeripheralEmulator::VirtioEmulation() {
  if (!virtio_write_) {
    return;
  }
  virtio_write_ = false;
  constexpr int kWordWidth = 4;
  if (kVirtioMmioQueueSel < virtio_address_ + virtio_width_ && virtio_address_ < kVirtioMmioQueueSel + kWordWidth) {
    const uint32_t queue_sel = memory_->Read32(kVirtioMmioQueueSel);
    const uint32_t queue_num_max = (queue_sel == 0) ? kQueueNumMax : 0;
    memory_->Write32(kVirtioMmioQueueMax, queue_num_max);
  }
  if (virtio_address_ + virtio_width_ <= kVirtioMmioQueueNotify ||
      kVirtioMmioQueueNotify + kWordWidth <= virtio_address_) {
    // If QUEUE_NOTIFY is not touched. End of the process.
    return;
  }
  // The rest processes the read/write request.
  uint32_t queue_number = memory_->Read32(kVirtioMmioQueueNotify);
  // For now, only 0th queue is available.
  assert(queue_number == 0);
  queue_num_ = memory_->Read32(kVirtioMmioQueueNum);
  assert(queue_num_ <= kQueueNumMax);
  const int kPageSize = memory_->Read32(kVirtioMmioPageSize);
  assert(kPageSize == 4096);
  const uint64_t kQueueAddress = memory_->Read32(kVirtioMmioQueuePfn) * kPageSize;
  VirtioDiskAccess(kQueueAddress);
  // Fire an interrupt.
  // New standard has a way to suspend interrupt until index reaches a certain value, but not supported in xv6.
  constexpr int kVirtioIrq = 1;
  memory_->Write32(kPlicClaimAddress, kVirtioIrq);
  virtio_interrupt_ = true;
}

void PeripheralEmulator::read_desc(VRingDesc *desc, uint64_t desc_address, uint16_t desc_index) const {
  constexpr int kVRingSize = 16;
  // std::cerr << "desc base address = " << std::hex << desc_address << std::endl;
  // std::cerr << "desc index = " << std::dec << desc_index << std::endl;
  uint64_t address = desc_address + desc_index * kVRingSize;
  // std::cerr << "desc address = " << std::hex << address << std::endl;
  desc->addr = memory_->Read64(address);
  desc->len = memory_->Read32(address + 8);
  desc->flags = memory_->Read16(address + 12);
  desc->next = memory_->Read16(address + 14);
  // std::cerr << "desc->addr = " << std::hex << desc->addr << std::endl;
  // std::cerr << "desc->len = " << std::dec << desc->len << std::endl;
  // std::cerr << "desc->flags = " << desc->flags << std::endl;
  // std::cerr << "desc->next = " << desc->next << std::endl;
}

void PeripheralEmulator::VirtioDiskAccess(uint64_t queue_address) {
  uint64_t desc_address = queue_address;
  constexpr uint32_t kDescSizeBytes = 16;
  uint64_t avail_address = queue_address + queue_num_ * kDescSizeBytes;
  constexpr int kPageSize = 4096;
  // used_address is at the page boundary.
  uint64_t used_address = ((avail_address + 2 * (2 + queue_num_) + kPageSize - 1) / kPageSize) * kPageSize;
  // std::cerr << "queue_address = " << std::hex << queue_address << std::endl;
  // std::cerr << "avail_address = " << std::hex << avail_address << std::endl;
  // std::cerr << "used_address = " << std::hex << used_address << std::endl;
  uint16_t desc_index = get_desc_index(avail_address);
  process_disc_access(desc_address, desc_index);
  process_used_buffer(used_address, desc_index);
}

uint16_t PeripheralEmulator::get_desc_index(
    uint64_t avail_address) const {  // Second word (16 bit) in Available Ring shows the next index.
  uint16_t index = memory_->Read16(avail_address + 2) % queue_num_;
  // std::cerr << "avail_index = " << index << std::endl;
  assert(index < queue_num_);
  uint16_t desc_index = memory_->Read16(avail_address + 4 + index * 2);
  return desc_index;
}

void PeripheralEmulator::process_disc_access(uint64_t desc_address, int desc_index) {
  constexpr uint8_t kOk = 0;

  VRingDesc desc;
  virtio_blk_outhdr outhdr;
  uint64_t buffer_address;
  uint32_t len;
  read_desc(&desc, desc_address, desc_index);
  read_outhdr(&outhdr, desc.addr);
  bool write_access = outhdr.type == 1;
  uint64_t sector = outhdr.sector;
  if ((desc.flags & 0b01) != 1) {
    // The first desc always need the next desc.
    std::cerr << "No next desc in the first entry" << std::endl;
    goto ERROR;
  }
  desc_index = desc.next;
  read_desc(&desc, desc_address, desc_index);
  buffer_address = desc.addr;
  if (((desc.flags & 0b10) == 0) != write_access) {
    // The read/write descriptions in outhdr and descriptor should match.
    // Note that "device write" is "disk read."
    std::cerr << "desc.flags = " << desc.flags << std::endl;
    std::cerr << "write_access = " << write_access << std::endl;
    goto ERROR;
  }
  len = desc.len;
  disc_access(sector, buffer_address, len, write_access);
  // Write to status. OK = 0.
  desc_index = desc.next;
  read_desc(&desc, desc_address, desc_index);
  if (desc.len != 1 || (desc.flags & 0b11) != 0b10) {
    // write access, and there's no next descriptor.
    std::cerr << "desc.len = " << desc.len << std::endl;
    std::cerr << "desc.flags = " << desc.flags << std::endl;
    goto ERROR;
  }
  buffer_address = desc.addr;
  memory_->WriteByte(buffer_address, kOk);
  return;
ERROR:
  // TODO: Add error handling.
  assert(false);
  return;
}

void PeripheralEmulator::read_outhdr(virtio_blk_outhdr *outhdr, uint64_t outhdr_address) const {
  // std::cerr << "virtio_blk_outhdr address = " << std::hex << outhdr_address << std::endl;
  outhdr->type = memory_->Read32(outhdr_address);
  outhdr->reserved = memory_->Read32(outhdr_address + 4);
  outhdr->sector = memory_->Read64(outhdr_address + 8);
  // std::cerr << "virtio_blk_outhdr.type = " << outhdr->type << std::endl;
  // std::cerr << "virtio_blk_outhdr.sector = " << outhdr->sector << std::endl;
}

void PeripheralEmulator::disc_access(uint64_t sector, uint64_t buffer_address, uint32_t len, bool write) {
  uint64_t kSectorAddress = sector * kSectorSize;
  // std::cerr << (write ? "Disk Write: " : "Disk Read: ");
  // std::cerr << "sector = " << std::hex << sector << ", size = " << std::dec << len << std::endl;
  if (write) {
    for (uint64_t offset = 0; offset < len; ++offset) {
      (*disk_image_)[kSectorAddress + offset] = memory_->ReadByte(buffer_address + offset);
    }
  } else {
    for (uint64_t offset = 0; offset < len; ++offset) {
      memory_->WriteByte(buffer_address + offset, (*disk_image_)[kSectorAddress + offset]);
    }
  }
}

void PeripheralEmulator::process_used_buffer(uint64_t used_buffer_address, uint16_t index) {
//  uint16_t flag = memory_->Read16(used_buffer_address);
  // TODO: Add check of flag.
  uint16_t current_used_index = memory_->Read16(used_buffer_address + 2);
  memory_->Write32(used_buffer_address + 4 + current_used_index * 8, index);
  memory_->Write32(used_buffer_address + 4 + current_used_index * 8 + 4, 3);
  current_used_index = current_used_index + 1;
  memory_->Write16(used_buffer_address + 2, current_used_index);
}

}  // namespace RISCV_EMULATOR