//
// Created by moiz on 1/19/20.
//

#ifndef MEMORY_WRAPPER_H
#define MEMORY_WRAPPER_H


#include <vector>
#include <cstdint>
#include <array>

class memory_wrapper_iterator; // Forward declaration.
class memory_wrapper;

constexpr int generate_mask(const int bits) {
  int mask = 0;
  for (int i = 0; i < bits; i++) {
    mask = (mask << 1) | 0b1;
  }
  return mask;
}

class memory_wrapper {
  static constexpr int kTotalBits = 32;
  static constexpr int kOffsetBits= 20;
  static constexpr int kOffsetMask= generate_mask(kOffsetBits);
  static constexpr int kEntryBits = kTotalBits - kOffsetBits;
  static constexpr int kEntryMask = generate_mask(kEntryBits);
  static constexpr int kMapEntry = 1 << kEntryBits;
  static constexpr size_t kMaxAddress = ((1ull << kTotalBits) - 1);
public:
  uint8_t &operator[]( size_t i);
  const uint8_t operator[]( size_t i) const;
  const uint32_t read32(size_t i) const;
  void write32(size_t i, uint32_t value);
  memory_wrapper_iterator begin();
  memory_wrapper_iterator end();
  bool operator==(memory_wrapper &r);
  bool operator!=(memory_wrapper &r);
private:
  bool check_range(int entry) const;
  std::array<std::vector<uint8_t>, kMapEntry> mapping;
};

class memory_wrapper_iterator{
public:
  using iterator = memory_wrapper_iterator;
  memory_wrapper_iterator(memory_wrapper &, size_t = 0);

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
  class memory_wrapper *mw;
};

uint32_t load_wd(const memory_wrapper_iterator &&address);
void store_wd(memory_wrapper_iterator &&address, uint32_t data, int width = 32);

#endif //MEMORY_WRAPPER_H
