//
// Created by moiz on 1/19/20.
//

#include <iostream>
#include <algorithm>
#include <random>
#include <functional>
#include "memory_wrapper.h"

int GetHash32(uint32_t val) {
  static std::hash<uint32_t> h;
  return h(val);
}

int GetHash8(uint32_t val) {
  return GetHash32(val) & 0xFF;
}

bool MemoryWrapperTest(size_t start, size_t end, int val) {
  bool result = true;
  MemoryWrapper mw;
  for (size_t j = start; j < end; j++) {
    mw[j] = GetHash8(j + val);
  }

  for (size_t j = start; j < end && result; j++) {
    bool local_result = mw[j] == GetHash8(j + val);
    if (!local_result) {
      std::cout << "mw[" << j << "] = " << static_cast<int>(mw[j]) << ", expectation = " << GetHash8(j)
      << std::endl;
    }
    result &= local_result;
  }
  if (!result) {
    std::cout << "Memory Wrapper Test failed. (" << start << ", " << end << ")\n";
  }
  return result;
}

bool MemoryWrapperIteratorTest1(size_t start, size_t end, int val) {
  // Test for *, ++, <, != operation
  bool result = true;
  MemoryWrapper mw;
  auto b = mw.begin() + start;
  auto e = mw.begin() + end;
  int value = val;
  for(auto i = b; i < e; i++, value++) {
    *i = GetHash8(value);
  }

  value = val;
  int index = 0;
  for (auto i = b; i != e && result; i++, value++, index++) {
    bool local_result = *i == GetHash8(value);
    if (!local_result) {
      std::cout << "At " << index << ", *i = " << static_cast<int>(*i) << ", expectation = " << GetHash8(value)
                << std::endl;
    }
    result &= local_result;
  }
  if (!result) {
    std::cout << "Memory Wrapper Iterator test 1 failed. (" << start << ", " << end << ")" << std::endl;
  }
  return result;
}

bool MemoryWrapperIteratorTest2(size_t start, size_t end, int val) {
  // Test for [] operation
  bool result = true;
  MemoryWrapper mw;
  auto i = mw.begin();
  for( size_t index = start; index < end; index++) {
    i[index] = GetHash8(index + val);
  }

  for( size_t index = start; index < end && result; index++) {
    bool local_result = i[index] == GetHash8(index + val);
    if (!local_result) {
      std::cout << "i[" << index << "] = " << static_cast<int>(i[index]) << ", expectation = " << GetHash8(index + val)
                << std::endl;
    }
    result &= local_result;
  }
  if (!result) {
    std::cout << "Memory wrapper iterator test 2 failed. (" << start << ", " << end << ")" << std::endl;
  }
  return result;
}

bool MemoryWrapperIteratorTest3(size_t start, size_t end, int val) {
  // Test for +, -. --, >, operation
  bool result = true;
  MemoryWrapper mw;
  auto i = mw.begin();
  for(size_t index = start; index < end; index++) {
    *(i + index) = GetHash8(index + val);
  }

  auto b = mw.begin() + start;
  int index = end - 1;
  for (auto e = mw.begin() + end - 1; e > b && result; e--, index--) {
    bool local_result = *e == GetHash8(index + val);
    if (!local_result) {
      std::cout << "At " << index << ", *e = " << static_cast<int>(*e) << ", expectation = " << GetHash8(index + val)
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

void InitRandom() {
  rnd.seed(kSeed);
}

constexpr size_t kMaxTestSize = 16 * 1024 * 1024;
constexpr size_t kSmallTestSize = 1 * 1024 * 1024;
constexpr int kTestCycle = 8;
constexpr int kSmallTestCycle = 100;

bool RunTestSingle(size_t start, size_t end, int val, bool verbose = false) {
  bool result = true;
  if (verbose) {
    std::cout << "Start: " << start << ", end: " << end << std::endl;
  }
  result = result && MemoryWrapperTest(start, end, val);
  result = result && MemoryWrapperIteratorTest1(start, end, val);
  result = result && MemoryWrapperIteratorTest2(start, end, val);
  result = result && MemoryWrapperIteratorTest3(start, end, val);
  return result;
}

bool RunTests(int test_cycle, size_t test_size, bool verbose = false) {
  bool result = true;
  for (int i = 0; i < test_cycle && result; i++) {
    InitRandom();
    size_t start = 0, end = 0;
    int val = 0;
    while (end <= start || (end - start) > test_size) {
      start = rand();
      end = rand();
      val = rand();
    }
    result = result && RunTestSingle(start, end, val, verbose);
    if (i % 10 == 0 && i > 0) {
      std::cout << i << " tests finished." << std::endl;
    }
  }
  return result;
}

bool Run32bitTest(const size_t start, const size_t end, const size_t val, const bool verbose) {
  if (verbose) {
    std::cout << "Start: " << start << ", end: " << end << std::endl;
  }
  int result = true;
  MemoryWrapper mw;
  for (size_t i = start; i < end; i += 4) {
    mw.Write32(i, GetHash32(i + val));
  }
  for (size_t i = start; i < end && result; i += 4) {
    uint32_t expectation = GetHash32(i + val);
    uint32_t read_value = mw.Read32(i);
    result &= read_value == expectation;
    if (!result) {
      std::cout << "At i = " << i << ", expectation = " << std::hex << expectation << ", actual = " << read_value << std::endl;
    }
  }
  return result;
}

bool Run32bitTest(int test_cycle, size_t test_size, bool verbose = false) {
  bool result = true;
  for (int i = 0; i < test_cycle && result; i++) {
    InitRandom();
    size_t start = 0, end = 0;
    int val = 0;
    while (end <= start || (end - start) > test_size) {
      start = rand();
      end = rand();
      val = rand();
    }
    result = result && Run32bitTest(start, end, val, verbose);
    if (i % 10 == 0 && i > 0) {
      std::cout << i << " tests finished." << std::endl;
    }
  }
  return result;
}

int main() {
  bool result = RunTests(kSmallTestCycle, kSmallTestSize, false);
  if (result) {
    std::cout << kSmallTestCycle << " small tests passed." << std::endl;
  } else {
    std::cout << "Small test failed." << std::endl;
  }
  result = result && RunTests(kTestCycle, kMaxTestSize, true);
  if (result) {
    std::cout << "memory_wrapper test pass." << std::endl;
  } else {
    std::cout << "memory_wrapper test fail." << std::endl;
  }
  result = result && Run32bitTest(kSmallTestCycle, kSmallTestSize, false);
  if (result) {
    std::cout << "32bit read/write test pass." << std::endl;
  } else {
    std::cout << "32bit read/write test fail." << std::endl;
  }
  return result ? 0 : 1;
}