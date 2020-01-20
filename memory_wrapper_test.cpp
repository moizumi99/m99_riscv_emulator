//
// Created by moiz on 1/19/20.
//

#include <iostream>
#include <algorithm>
#include <array>
#include <random>
#include "memory_wrapper.h"

bool memory_wrapper_test(size_t start, size_t end, int val) {
  bool result = true;
  memory_wrapper mw;
  int value = val;
  for (size_t j = start; j < end; j++, value++) {
    mw[j] = value & 0xFF;
  }

  value = val;
  for (size_t j = start; j < end && result; j++, value++) {
    bool local_result = mw[j] == (value & 0xFF);
    if (!local_result) {
      std::cout << "mw[" << j << "] = " << static_cast<int>(mw[j]) << ", expectation = " << (value & 0xFF)
      << std::endl;
    }
    result &= local_result;
  }
  if (!result) {
    std::cout << "Memory Wrapper Test failed. (" << start << ", " << end << ")\n";
  }
  return result;
}

bool memory_wrapper_iterator_test_1(size_t start, size_t end, int val) {
  // Test for *, ++, <, != operation
  bool result = true;
  memory_wrapper mw;
  auto b = mw.begin() + start;
  auto e = mw.begin() + end;
  int value = val;
  for(auto i = b; i < e; i++, value++) {
    *i = value & 0xff;
  }

  value = val;
  int index = 0;
  for (auto i = b; i != e && result; i++, value++, index++) {
    bool local_result = *i == (value & 0xff);
    if (!local_result) {
      std::cout << "At " << index << ", *i = " << static_cast<int>(*i) << ", expectation = " << (value & 0xff)
                << std::endl;
    }
    result &= local_result;
  }
  if (!result) {
    std::cout << "Memory Wrapper Iterator test 1 failed. (" << start << ", " << end << ")" << std::endl;
  }
  return result;
}

bool memory_wrapper_iterator_test_2(size_t start, size_t end, int val) {
  // Test for [] operation
  bool result = true;
  memory_wrapper mw;
  auto i = mw.begin();
  int value = val;
  for( size_t index = start; index < end; index++, value++) {
    i[index] = value & 0xFF;
  }

  value = val;
  for( size_t index = start; index < end && result; index++, value++) {
    bool local_result = i[index] == (value & 0xff);
    if (!local_result) {
      std::cout << "i[" << index << "] = " << static_cast<int>(i[index]) << ", expectation = " << (value & 0xff)
                << std::endl;
    }
    result &= local_result;
  }
  if (!result) {
    std::cout << "Memory wrapper iterator test 2 failed. (" << start << ", " << end << ")" << std::endl;
  }
}

bool memory_wrapper_iterator_test_3(size_t start, size_t end, int val) {
  // Test for +, -. --, >, operation
  bool result = true;
  memory_wrapper mw;
  auto i = mw.begin();
  int value = val;
  for(size_t index = start; index < end; index++, value++) {
    *(i + index) = value & 0xFF;
  }

  auto b = mw.begin() + start;
  value = val + (end - start) - 1;
  int index = end - 1;
  for (auto e = mw.begin() + end - 1; e > b && result; e--, value--, index--) {
    bool local_result = *e == (value & 0xff);
    if (!local_result) {
      std::cout << "At " << index << ", *e = " << static_cast<int>(*e) << ", expectation = " << (value & 0xff)
                << std::endl;
    }
    result &= local_result;
  }
  if (!result) {
    std::cout << "Memory Wrapper Iterator test 3 failed. (" << start << ", " << end << ")" << std::endl;
  }
  return result;
}

std::mt19937 rnd;
constexpr int kSeed = 155719;

void init_random() {
  rnd.seed(kSeed);
}

constexpr size_t kMaxTestSize = 16 * 1024 * 1024;
constexpr int kTestCycle = 10;

bool run_test_single(size_t start, size_t end, int val) {
  bool result = true;
  std::cout << "Start: " << start << ", end: " << end << std::endl;
  result = result && memory_wrapper_test(start, end, val);
  result = result && memory_wrapper_iterator_test_1(start, end, val);
  result = result && memory_wrapper_iterator_test_2(start, end, val);
  result = result && memory_wrapper_iterator_test_3(start, end, val);
  return result;
}

bool run_test() {
  bool result = true;
  for (int i = 0; i < kTestCycle && result; i++) {
    init_random();
    size_t start = 0, end = 0;
    int val = 0;
    while (end <= start || (end - start) > kMaxTestSize) {
      start = rand();
      end = rand();
      val = rand() & 0xFF;
    }
    result = result && run_test_single(start, end, val);
  }
  return result;
}


int main() {
  bool result = run_test();
  if (result) {
    std::cout << "memory_wrapper test pass.";
  } else {
    std::cout << "memory_wrapper test fail.";
  }
  return result ? 0 : 1;
}