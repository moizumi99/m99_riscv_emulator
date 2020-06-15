//
// Created by moiz on 1/19/20.
//
#include <stdexcept>
#include <cassert>
#include "memory_wrapper.h"

namespace RISCV_EMULATOR {

bool MemoryWrapper::CheckRange(int entry) const {
  if (entry < 0 || entry >= kMapEntry) {
    throw std::out_of_range("Memory wrapper size out of range.");
  }
  return !mapping[entry].empty();
}

const uint8_t MemoryWrapper::ReadByte(size_t i) const {
  uint64_t dram_address = (i >> kWordBits) << kWordBits;
  uint64_t read_data = Read32(dram_address);
  int byte_offset = i & 0b11;
  read_data = read_data >> byte_offset * 8;
  return static_cast<uint8_t>(read_data & 0xFF);
}

void MemoryWrapper::WriteByte(size_t i, uint8_t data) {
  std::array<uint64_t, 4> mask = {
    0xFFFFFF00,
    0xFFFF00FF,
    0xFF00FFFF,
    0x00FFFFFF,
  };
  uint64_t dram_address = (i >> kWordBits) << kWordBits;
  uint64_t original_data = Read32(dram_address);
  uint64_t write_data = data;
  int byte_offset = i & 0b11;
  write_data = (original_data & mask[byte_offset]) | (write_data << byte_offset * 8);
  Write32(dram_address, write_data);
}

const uint32_t MemoryWrapper::Read32(size_t i) const {
  assert((i & 0b11) == 0);
  int entry = (i >> kOffsetBits) & kEntryMask;
  uint64_t read_data = 0;
  if (CheckRange(entry)) {
    int offset = i & kOffsetMask;
    int word_offset = offset >> kWordBits;
    read_data = mapping[entry][word_offset];
  }
  return read_data;
}

void MemoryWrapper::Write32(size_t i, uint32_t value) {
  assert((i & 0b11) == 0);
  int entry = (i >> kOffsetBits) & kEntryMask;
  if (!CheckRange(entry)) {
    mapping[entry].resize(1 << (kOffsetBits - kWordBits));
  }
  int offset = i & kOffsetMask;
  int word_offset = offset >> kWordBits;
  mapping[entry][word_offset] = value;
}

const uint64_t MemoryWrapper::Read64(size_t i) const {
  assert((i & 0b111) == 0);
  uint64_t dram_address = (i >> kWordBits) << kWordBits;
  uint64_t read_data_lower = Read32(dram_address);
  uint64_t read_data_upper = Read32(dram_address + 4);
  uint64_t read_data = read_data_lower | (read_data_upper << 32);
  return read_data;
}

void MemoryWrapper::Write64(size_t i, uint64_t value) {
  assert((i & 0b111) == 0);
  uint64_t dram_address = (i >> kWordBits) << kWordBits;
  Write32(dram_address, value & 0xFFFFFFFF);
  Write32(dram_address + 4, (value >> 32) & 0xFFFFFFFF);
}

bool MemoryWrapper::operator==(MemoryWrapper &r) {
  return mapping == r.mapping;
}

bool MemoryWrapper::operator!=(MemoryWrapper &r) {
  return mapping != r.mapping;
}

} // namespace RISCV_EMULATOR
