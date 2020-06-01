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

uint8_t &MemoryWrapper::operator[](size_t i) {
  int entry = (i >> kOffsetBits) & kEntryMask;
  int offset = i & kOffsetMask;
  if (!CheckRange(entry)) {
    mapping[entry].resize(1 << kOffsetBits, 0);
  };
  return mapping[entry][offset];
}

const uint8_t MemoryWrapper::operator[](size_t i) const {
  int entry = (i >> kOffsetBits) & kEntryMask;
  if (!CheckRange(entry)) {
    return 0;
  }
  int offset = i & kOffsetMask;
  return mapping[entry][offset];
}

const uint32_t MemoryWrapper::Read32(size_t i) const {
  return (*this)[i] | ((*this)[i + 1] << 8) | ((*this)[i + 2] << 16) |
         ((*this)[i + 3] << 24);
}

const uint64_t MemoryWrapper::Read64(size_t i) const {
  uint64_t read_data = 0;
  for (int offset = 0; offset < 8; offset++) {
    read_data |= static_cast<uint64_t>((*this)[i + offset]) << offset * 8;
  }
  return read_data;
}

void MemoryWrapper::Write32(size_t i, uint32_t value) {
  (*this)[i] = value & 0xFF;
  (*this)[i + 1] = (value >> 8) & 0xFF;
  (*this)[i + 2] = (value >> 16) & 0xFF;
  (*this)[i + 3] = (value >> 24) & 0xFF;
}

void MemoryWrapper::Write64(size_t i, uint64_t value) {
  (*this)[i] = value & 0xFF;
  (*this)[i + 1] = (value >> 8) & 0xFF;
  (*this)[i + 2] = (value >> 16) & 0xFF;
  (*this)[i + 3] = (value >> 24) & 0xFF;
  (*this)[i + 4] = (value >> 32) & 0xFF;
  (*this)[i + 5] = (value >> 40) & 0xFF;
  (*this)[i + 6] = (value >> 48) & 0xFF;
  (*this)[i + 7] = (value >> 56) & 0xFF;
}

bool MemoryWrapper::operator==(MemoryWrapper &r) {
  return mapping == r.mapping;
}

bool MemoryWrapper::operator!=(MemoryWrapper &r) {
  return mapping != r.mapping;
}

MemoryWrapperIterator MemoryWrapper::begin() {
  return MemoryWrapperIterator(*this, 0);
}

MemoryWrapperIterator MemoryWrapper::end() {
  return MemoryWrapperIterator(*this, kMaxAddress);
}

// Memory wrapper iterator definition starts here.
MemoryWrapperIterator::MemoryWrapperIterator(MemoryWrapper &m, size_t p) : pos(p),
                                                                           mw(
                                                                           &m) {}

uint8_t &MemoryWrapperIterator::operator*() {
  return (*this->mw)[pos];
}

size_t MemoryWrapperIterator::GetAddress() {
  return pos;
}

const uint8_t MemoryWrapperIterator::operator*() const {
  return (*this->mw)[pos];
}

uint8_t &MemoryWrapperIterator::operator[](size_t n) {
  return (*this->mw)[pos + n];
}

const uint8_t MemoryWrapperIterator::operator[](size_t n) const {
  return (*this->mw)[pos + n];
}

MemoryWrapperIterator &MemoryWrapperIterator::operator++() {
  ++pos;
  return *this;
}

MemoryWrapperIterator MemoryWrapperIterator::operator++(int) {
  MemoryWrapperIterator copy = *this;
  ++pos;
  return copy;
}

MemoryWrapperIterator &MemoryWrapperIterator::operator--() {
  --pos;
  return *this;
}

MemoryWrapperIterator MemoryWrapperIterator::operator--(int) {
  MemoryWrapperIterator copy = *this;
  --pos;
  return copy;
}

MemoryWrapperIterator &MemoryWrapperIterator::operator+=(size_t n) {
  pos += n;
  return *this;
}

MemoryWrapperIterator &MemoryWrapperIterator::operator-=(size_t n) {
  pos -= n;
  return *this;
}

MemoryWrapperIterator MemoryWrapperIterator::operator+(size_t n) {
  MemoryWrapperIterator copy = *this;
  copy += n;
  return copy;
}

MemoryWrapperIterator MemoryWrapperIterator::operator-(size_t n) {
  MemoryWrapperIterator copy = *this;
  copy -= n;
  return copy;
}


bool MemoryWrapperIterator::operator==(const iterator &r) {
  return (mw == r.mw && pos == r.pos);
}

bool MemoryWrapperIterator::operator!=(const iterator &r) {
  return (mw != r.mw || pos != r.pos);
}

bool MemoryWrapperIterator::operator<(const iterator &r) {
  return (mw == r.mw && pos < r.pos);
}

bool MemoryWrapperIterator::operator>(const iterator &r) {
  return (mw == r.mw && pos > r.pos);
}

bool MemoryWrapperIterator::operator<=(const iterator &r) {
  return (mw == r.mw && pos <= r.pos);
}

bool MemoryWrapperIterator::operator>=(const iterator &r) {
  return (mw == r.mw && pos >= r.pos);
}

uint32_t LoadWd(const MemoryWrapperIterator &&address) {
  return address[0] | (address[1] << 8) | (address[2] << 16) |
         (address[3] << 24);
}

void StoreWd(MemoryWrapperIterator &&address, uint32_t data, int width) {
  switch (width) {
    case 32:
      address[2] = (data >> 16) & 0xFF;
      address[3] = (data >> 24) & 0xFF;
    case 16:
      address[1] = (data >> 8) & 0xFF;
    case 8:
      address[0] = data & 0xFF;
      break;
    default:
      throw std::invalid_argument("Store width is not 8, 16, or 32.");
  }
}

} // namespace RISCV_EMULATOR
