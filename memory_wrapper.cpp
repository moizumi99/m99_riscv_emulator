//
// Created by moiz on 1/19/20.
//
#include <stdexcept>
#include "memory_wrapper.h"

void memory_wrapper::check_range(size_t i) {
  if (i >= kMaxEntry) {
    throw std::out_of_range("Memory wrapper size out of range.");
  }
  int entry = (i >> kOffsetBits) & kEntryMask;
  int offset = i & kOffsetMask;
  if (mapping[entry].empty()) {
    mapping[entry].resize(1 << kOffsetBits, 0);
  }
}

uint8_t &memory_wrapper::operator[]( size_t i) {
  check_range(i);
  int entry = (i >> kOffsetBits) & kEntryMask;
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
  return memory_wrapper_iterator(*this, kMaxEntry);
}

// Memory wrapper iterator definition starts here.
memory_wrapper_iterator::memory_wrapper_iterator(memory_wrapper &m, size_t p): mw(m), pos(p) {}

uint8_t& memory_wrapper_iterator::operator*() {
  return (this->mw)[pos];
}

const uint8_t& memory_wrapper_iterator::operator*() const {
  return (this->mw)[pos];
}

uint8_t &memory_wrapper_iterator::operator[](size_t n) {
  return (this->mw)[pos + n];
}

const uint8_t &memory_wrapper_iterator::operator[](size_t n) const {
  return (this->mw)[pos + n];
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
  return pos == r.pos;
}

bool memory_wrapper_iterator::operator!=(const iterator &r) {
  return pos != r.pos;
}

bool memory_wrapper_iterator::operator<(const iterator &r) {
  return pos < r.pos;
}

bool memory_wrapper_iterator::operator>(const iterator &r) {
  return pos > r.pos;
}

bool memory_wrapper_iterator::operator<=(const iterator &r) {
  return pos <= r.pos;
}

bool memory_wrapper_iterator::operator>=(const iterator &r) {
  return pos >= r.pos;
}




