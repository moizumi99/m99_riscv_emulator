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

class memory_wrapper {
  static constexpr int kTotalBits = 32;
  static constexpr int kOffsetBits= 20;
  static constexpr int kOffsetMask= 0x0FFFFF;
  static constexpr int kEntryBits = kTotalBits - kOffsetBits;
  static constexpr int kEntryMask = 0x0FFF;
  static constexpr int kMapEntry = 1 << kEntryBits;
  static constexpr size_t kMaxEntry = 1l << kTotalBits;
public:
  uint8_t &operator[]( size_t i);
  memory_wrapper_iterator begin();
  memory_wrapper_iterator end();
  bool operator==(memory_wrapper &r);
  bool operator!=(memory_wrapper &r);
private:
  void check_range(size_t i);
  std::array<std::vector<uint8_t>, kMapEntry> mapping;
};

class memory_wrapper_iterator{
public:
  using iterator = memory_wrapper_iterator;
  memory_wrapper_iterator(memory_wrapper &, size_t = 0);

  uint8_t &operator*();
  const uint8_t &operator*() const;
  uint8_t &operator->() = delete;
  const uint8_t &operator->() const = delete;
  uint8_t &operator[](size_t);
  const uint8_t &operator[](size_t) const;
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
  class memory_wrapper &mw;
};


#endif //MEMORY_WRAPPER_H
