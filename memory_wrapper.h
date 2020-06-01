//
// Created by moiz on 1/19/20.
//

#ifndef MEMORY_WRAPPER_H
#define MEMORY_WRAPPER_H


#include <vector>
#include <cstdint>
#include <array>

namespace RISCV_EMULATOR {

class MemoryWrapperIterator; // Forward declaration.
class MemoryWrapper;

constexpr int GenerateBitMask(const int bits) {
  int mask = 0;
  for (int i = 0; i < bits; i++) {
    mask = (mask << 1) | 0b1;
  }
  return mask;
}

class MemoryWrapper {
  static constexpr int kTotalBits = 32;
  static constexpr int kOffsetBits = 20;
  static constexpr int kOffsetMask = GenerateBitMask(kOffsetBits);
  static constexpr int kEntryBits = kTotalBits - kOffsetBits;
  static constexpr int kEntryMask = GenerateBitMask(kEntryBits);
  static constexpr int kMapEntry = 1 << kEntryBits;
  static constexpr size_t kMaxAddress = ((1ull << kTotalBits) - 1);
public:
  uint8_t &operator[](size_t i);

  const uint8_t operator[](size_t i) const;

  const uint32_t Read32(size_t i) const;

  const uint64_t Read64(size_t i) const;

  void Write32(size_t i, uint32_t value);

  void Write64(size_t i, uint64_t value);

  MemoryWrapperIterator begin();

  MemoryWrapperIterator end();

  bool operator==(MemoryWrapper &r);

  bool operator!=(MemoryWrapper &r);

private:
  bool CheckRange(int entry) const;

  std::array<std::vector<uint8_t>, kMapEntry> mapping;
};

class MemoryWrapperIterator {
public:
  using iterator = MemoryWrapperIterator;

  MemoryWrapperIterator(MemoryWrapper &, size_t = 0);

  size_t GetAddress();

  uint8_t &operator*();

  const uint8_t operator*() const;

  uint8_t &operator->() = delete;

  const uint8_t &operator->() const = delete;

  uint8_t &operator[](size_t);

  const uint8_t operator[](size_t) const;

  iterator &operator++();

  iterator operator++(int);

  iterator &operator--();

  iterator operator--(int);

  iterator &operator+=(size_t);

  iterator &operator-=(size_t);

  iterator operator+(size_t);

  iterator operator-(size_t);

  bool operator==(const iterator &r);

  bool operator!=(const iterator &r);

  bool operator<(const iterator &r);

  bool operator>(const iterator &r);

  bool operator<=(const iterator &r);

  bool operator>=(const iterator &r);

private:
  size_t pos;

  class MemoryWrapper *mw;
};

uint32_t LoadWd(const MemoryWrapperIterator &&address);

void StoreWd(MemoryWrapperIterator &&address, uint32_t data, int width = 32);

} // namespace RISCV_EMULATOR

#endif //MEMORY_WRAPPER_H
