//
// Created by moiz on 1/19/20.
//
#include <stdexcept>
#include "memory_wrapper.h"

namespace RISCV_EMULATOR {

bool MemoryWrapper::CheckRange(int entry) const {
  if (entry < 0 || entry >= kMapEntry) {
    throw std::out_of_range("Memory wrapper size out of range.");
  }
  return !mapping[entry].empty();
}

void MemoryWrapper::WriteByte(size_t i, uint8_t data) {
  int entry = (i >> kOffsetBits) & kEntryMask;
  int offset = i & kOffsetMask;
  if (!CheckRange(entry)) {
    mapping[entry].resize(1 << kOffsetBits, 0);
  };
  mapping[entry][offset] = data;
}

const uint8_t MemoryWrapper::ReadByte(size_t i) const {
  int entry = (i >> kOffsetBits) & kEntryMask;
  if (!CheckRange(entry)) {
    return 0;
  }
  int offset = i & kOffsetMask;
  return mapping[entry][offset];
}


const uint32_t MemoryWrapper::Read32(size_t i) const {
  return ReadByte(i) | (ReadByte(i + 1) << 8) | (ReadByte(i + 2) << 16) |
         (ReadByte(i + 3) << 24);
}

const uint64_t MemoryWrapper::Read64(size_t i) const {
  uint64_t read_data = 0;
  for (int offset = 0; offset < 8; offset++) {
    read_data |= static_cast<uint64_t>(ReadByte(i + offset)) << offset * 8;
  }
  return read_data;
}

void MemoryWrapper::Write32(size_t i, uint32_t value) {
  WriteByte(i, value & 0xFF);
  WriteByte(i + 1, (value >> 8) & 0xFF);
  WriteByte(i + 2, (value >> 16) & 0xFF);
  WriteByte(i + 3, (value >> 24) & 0xFF);
}

void MemoryWrapper::Write64(size_t i, uint64_t value) {
  for (int offset = 0; offset < 8; ++offset) {
    WriteByte(i + offset, (value >> offset * 8) & 0xFF);
  }
}

bool MemoryWrapper::operator==(MemoryWrapper &r) {
  return mapping == r.mapping;
}

bool MemoryWrapper::operator!=(MemoryWrapper &r) {
  return mapping != r.mapping;
}

} // namespace RISCV_EMULATOR
