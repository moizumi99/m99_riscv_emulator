//
// Created by moiz on 1/19/20.
//
#include <stdexcept>
#include "memory_wrapper.h"

bool memory_wrapper::check_range(int entry) const {
  if (entry < 0 || entry >= kMapEntry) {
    throw std::out_of_range("Memory wrapper size out of range.");
  }
  return !mapping[entry].empty();
}

uint8_t &memory_wrapper::operator[]( size_t i) {
  int entry = (i >> kOffsetBits) & kEntryMask;
  int offset = i & kOffsetMask;
  if (!check_range(entry)) {
    mapping[entry].resize(1 << kOffsetBits, 0);
  };
 return mapping[entry][offset];
}

const uint8_t memory_wrapper::operator[]( size_t i) const {
  int entry = (i >> kOffsetBits) & kEntryMask;
  if (!check_range(entry)) {
    return 0;
  }
  int offset = i & kOffsetMask;
  return mapping[entry][offset];
}

const uint32_t memory_wrapper::read32(size_t i) const {
  return (*this)[i] | ((*this)[i + 1] << 8) | ((*this)[i + 2] << 16) | ((*this)[i + 3] << 24);
}

void memory_wrapper::write32(size_t i, uint32_t value) {
  (*this)[i] = value & 0xFF;
  (*this)[i + 1] = (value >> 8) & 0xFF;
  (*this)[i + 2] = (value >> 16) & 0xFF;
  (*this)[i + 3] = (value >> 24) & 0xFF;
}

bool memory_wrapper::operator==(memory_wrapper &r) {
  return mapping == r.mapping;
}

bool memory_wrapper::operator!=(memory_wrapper &r) {
  return mapping != r.mapping;
}

memory_wrapper_iterator memory_wrapper::begin() {
  return memory_wrapper_iterator(*this, 0);
}

memory_wrapper_iterator memory_wrapper::end() {
  return memory_wrapper_iterator(*this, kMaxAddress);
}

// Memory wrapper iterator definition starts here.
memory_wrapper_iterator::memory_wrapper_iterator(memory_wrapper &m, size_t p): pos(p), mw(&m)  {}

uint8_t& memory_wrapper_iterator::operator*() {
  return (*this->mw)[pos];
}

const uint8_t memory_wrapper_iterator::operator*() const {
  return (*this->mw)[pos];
}

uint8_t &memory_wrapper_iterator::operator[](size_t n) {
  return (*this->mw)[pos + n];
}

const uint8_t memory_wrapper_iterator::operator[](size_t n) const {
  return (*this->mw)[pos + n];
}

memory_wrapper_iterator &memory_wrapper_iterator::operator++() {
  ++pos;
  return *this;
}

memory_wrapper_iterator memory_wrapper_iterator::operator++(int) {
  memory_wrapper_iterator copy = *this;
  ++pos;
  return copy;
}

memory_wrapper_iterator &memory_wrapper_iterator::operator--() {
  --pos;
  return *this;
}

memory_wrapper_iterator memory_wrapper_iterator::operator--(int) {
  memory_wrapper_iterator copy = *this;
  --pos;
  return copy;
}

memory_wrapper_iterator &memory_wrapper_iterator::operator+=(size_t n) {
  pos += n;
  return *this;
}

memory_wrapper_iterator &memory_wrapper_iterator::operator-=(size_t n) {
  pos -= n;
  return *this;
}

memory_wrapper_iterator memory_wrapper_iterator::operator+(size_t n) {
  memory_wrapper_iterator copy = *this;
  copy += n;
  return copy;
}

memory_wrapper_iterator memory_wrapper_iterator::operator-(size_t n) {
  memory_wrapper_iterator copy = *this;
  copy -= n;
  return copy;
}


bool memory_wrapper_iterator::operator==(const iterator &r) {
  return (mw == r.mw && pos == r.pos);
}

bool memory_wrapper_iterator::operator!=(const iterator &r) {
  return (mw != r.mw || pos != r.pos);
}

bool memory_wrapper_iterator::operator<(const iterator &r) {
  return (mw == r.mw && pos < r.pos);
}

bool memory_wrapper_iterator::operator>(const iterator &r) {
  return (mw == r.mw && pos > r.pos);
}

bool memory_wrapper_iterator::operator<=(const iterator &r) {
  return (mw == r.mw && pos <= r.pos);
}

bool memory_wrapper_iterator::operator>=(const iterator &r) {
  return (mw == r.mw && pos >= r.pos);
}

uint32_t load_wd(const memory_wrapper_iterator &&address) {
  return address[0] | (address[1] << 8) | (address[2] << 16) | (address[3] << 24);
}

void store_wd(memory_wrapper_iterator &&address, uint32_t data, int width) {
  switch(width) {
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



