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
memory_wrapper_iterator::memory_wrapper_iterator(memory_wrapper &m, size_t p): mw(&m), pos(p) {}

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




