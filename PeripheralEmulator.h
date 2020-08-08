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

struct VRingDesc {
  uint64_t addr;
  uint32_t len;
  uint16_t flags;
  uint16_t next;
};

struct virtio_blk_outhdr {
  uint32_t type;
  uint32_t reserved;
  uint64_t sector;
};

class PeripheralEmulator {
 public:
  // Memory Mapped Register Addresses.
  // Host Interface.
  static constexpr uint64_t kToHost0 = 0x80001000;
  static constexpr uint64_t kToHost1 = 0x80003000;
  static constexpr uint64_t kFromHost = 0x80001040;

  // UART.
  static constexpr uint64_t kUartBase = 0x10000000;
  static constexpr uint64_t kUartSize = 6;

  // Timer.
  static constexpr uint64_t kTimerBase = 0x2000000;
  static constexpr uint64_t kTimerCmp = kTimerBase + 0x4000;
  static constexpr uint64_t kTimerMtime = kTimerBase + 0xbff8;

  // Virtio Disk.
  static constexpr int kQueueNumMax = 8;
  static constexpr uint64_t kVirtioBase = 0x10001000;
  static constexpr uint64_t kVirtioEnd = kVirtioBase + 0x100 - 1;
  static constexpr uint64_t kVirtioMmioPageSize = kVirtioBase + 0x28;
  static constexpr uint64_t kVirtioMmioQueueSel = kVirtioBase + 0x30;
  static constexpr uint64_t kVirtioMmioQueueMax = kVirtioBase + 0x34;
  static constexpr uint64_t kVirtioMmioQueueNum = kVirtioBase + 0x38;
  static constexpr uint64_t kVirtioMmioQueuePfn = kVirtioBase + 0x40;
  static constexpr uint64_t kVirtioMmioQueueNotify = kVirtioBase + 0x50;

  PeripheralEmulator(int mxl);

  void SetMemory(std::shared_ptr<MemoryWrapper> memory);
  void Emulation();

  void Initialize();
  void CheckDeviceWrite(uint64_t address, int width, uint64_t data);
  void MemoryMappedValueUpdate();

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
  void SetDeviceEmulationEnable(bool enable) { device_emulation_enable = enable; }
  void UartInit();
  void UartEmulation();

  // Virtio Disk Emulation.
  void VirtioInit();
  void VirtioEmulation();
  void SetDiskImage(std::shared_ptr<std::vector<uint8_t>> disk_image);
  bool GetInterruptStatus() {return virtio_interrupt_; }
  void ClearInterruptStatus() { virtio_interrupt_ = false;}

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
  uint64_t elapsed_cycles_ = 0;
  uint64_t next_cycle_ = 0;
  bool timercmp_update_ = true;
  bool timer_interrupt_ = false;

  // Virtio
  static constexpr int kSectorSize = 512; // 1 sector = 512 bytes.
  std::shared_ptr<std::vector<uint8_t>> disk_image_;
  bool virtio_write_ = false;
  uint64_t virtio_address_ = 0;
  uint64_t virtio_width_ = 0;
  uint64_t virtio_data_ = 0;
  int queue_num_ = 8;
  bool virtio_interrupt_ = false;
  void VirtioDiskAccess(uint64_t queue_address);
  uint16_t get_desc_index(uint64_t avail_address) const;
  void read_desc(VRingDesc *desc, uint64_t desc_address, uint16_t desc_index) const;
  void read_outhdr(virtio_blk_outhdr *outhdr, uint64_t outhdr_address) const;
  void process_disc_access(uint64_t desc, int desc_index);
  void disc_access(uint64_t sector, uint64_t buffer_address, uint32_t len, bool write);
  void process_used_buffer(uint64_t used_buffer_address, uint16_t index);
};
}  // namespace RISCV_EMULATOR

#endif  // ASSEMBLER_TEST_PERIPHERALEMULATOR_H
