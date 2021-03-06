//
// Created by moiz on 1/19/20.
//

#include <iostream>
#include <algorithm>
#include <random>
#include <functional>
#include "memory_wrapper.h"

using namespace RISCV_EMULATOR;

namespace {
constexpr size_t kMaxTestSize = 16 * 1024 * 1024;
constexpr size_t kSmallTestSize = 1 * 1024 * 1024;
constexpr int kTestCycle = 8;
constexpr int kSmallTestCycle = 100;
} // namespace anonumous

namespace {

int GetHash64(uint32_t val) {
  static std::hash<uint64_t> h;
  return h(val);
}

int GetHash32(uint32_t val) {
  static std::hash<uint32_t> h;
  return h(val);
}

int GetHash8(uint32_t val) {
  return GetHash32(val) & 0xFF;
}

int GetHash16(uint32_t val) {
  return GetHash32(val) & 0xFFFF;
}

bool MemoryWrapperTest(size_t start, size_t end, int val) {
  bool result = true;
  MemoryWrapper mw;
  for (size_t j = start; j < end; j++) {
    mw.WriteByte(j, GetHash8(j + val));
  }

  for (size_t j = start; j < end && result; j++) {
    bool local_result = mw.ReadByte(j) == GetHash8(j + val);
    if (!local_result) {
      std::cout << "mw.Read(" << j << ") = " << static_cast<int>(mw.ReadByte(j))
                << ", expectation = " << GetHash8(j)
                << std::endl;
    }
    result &= local_result;
  }
  if (!result) {
    std::cout << "Memory Wrapper Test failed. (" << start << ", " << end
              << ")\n";
  }
  return result;
}


std::mt19937 rnd;
constexpr int kSeed = 155719;

void InitRandom() {
  rnd.seed(kSeed);
}


bool RunTestSingle(size_t start, size_t end, int val, bool verbose = false) {
  bool result = true;
  if (verbose) {
    std::cout << "Start: " << start << ", end: " << end << std::endl;
  }
  result = result && MemoryWrapperTest(start, end, val);
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

bool Run32bitTest(const size_t start, const size_t end, const size_t val,
                  const bool verbose) {
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
      std::cout << "At i = " << i << ", expectation = " << std::hex
                << expectation << ", actual = " << read_value << std::endl;
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
      start = (rand() >> 2) << 2;
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

bool Run64bitTest(const size_t start, const size_t end, const size_t val,
                  const bool verbose) {
  if (verbose) {
    std::cout << "Start: " << start << ", end: " << end << std::endl;
  }
  int result = true;
  MemoryWrapper mw;
  for (size_t i = start; i < end; i += 8) {
    mw.Write64(i, GetHash64(i + val));
  }
  for (size_t i = start; i < end && result; i += 8) {
    uint64_t expectation = GetHash64(i + val);
    uint64_t read_value = mw.Read64(i);
    result &= read_value == expectation;
    if (!result) {
      std::cout << "At i = " << i << ", expectation = " << std::hex
                << expectation << ", actual = " << read_value << std::endl;
    }
  }
  return result;
}

bool Run64bitTest(int test_cycle, size_t test_size, bool verbose = false) {
  bool result = true;
  for (int i = 0; i < test_cycle && result; i++) {
    InitRandom();
    size_t start = 0, end = 0;
    int val = 0;
    while (end <= start || (end - start) > test_size) {
      start = (rand() >> 3) << 3;
      end = rand();
      val = rand();
    }
    result = result && Run64bitTest(start, end, val, verbose);
    if (i % 10 == 0 && i > 0) {
      std::cout << i << " tests finished." << std::endl;
    }
  }
  return result;
}

bool Run16bitTest(const size_t start, const size_t end, const size_t val,
                  const bool verbose) {
  if (verbose) {
    std::cout << "Start: " << start << ", end: " << end << std::endl;
  }
  int result = true;
  MemoryWrapper mw;
  for (size_t i = start; i < end; i += 2) {
    uint16_t hash_value = GetHash16(i + val);
    mw.Write16(i, hash_value);
  }
  for (size_t i = start; i < end && result; i += 2) {
    uint16_t expectation = GetHash16(i + val);
    uint16_t read_value = mw.Read16(i);
    result &= read_value == expectation;
    if (!result) {
      std::cout << "At i = " << i << ", expectation = " << std::hex
                << expectation << ", actual = " << read_value << std::endl;
    }
  }
  return result;
}

bool Run16bitTest(int test_cycle, size_t test_size, bool verbose = false) {
  bool result = true;
  for (int i = 0; i < test_cycle && result; i++) {
    InitRandom();
    size_t start = 0, end = 0;
    int val = 0;
    while (end <= start || (end - start) > test_size) {
      start = (rand() >> 2) << 2;
      end = rand();
      val = rand();
    }
    result = result && Run16bitTest(start, end, val, verbose);
    if (i % 10 == 0 && i > 0) {
      std::cout << i << " tests finished." << std::endl;
    }
  }
  return result;
}

} // namespace anonymous

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
    std::cout << "32 bit read/write test pass." << std::endl;
  } else {
    std::cout << "32 bit read/write test fail." << std::endl;
  }
  result = result && Run64bitTest(kSmallTestCycle, kSmallTestSize, false);
  if (result) {
    std::cout << "64 bit read/write test pass." << std::endl;
  } else {
    std::cout << "64 bit read/write test fail." << std::endl;
  }
  result = result && Run16bitTest(kSmallTestCycle, kSmallTestSize, false);
  if (result) {
    std::cout << "16 bit read/write test pass." << std::endl;
  } else {
    std::cout << "16 bit read/write test fail." << std::endl;
  }
  return result ? 0 : 1;
}