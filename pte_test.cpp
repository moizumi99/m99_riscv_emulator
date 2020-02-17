//
// Created by moiz on 2/16/20.
//

#include "pte.h"

#include <random>
#include <iostream>
#include <bitset>

constexpr int kTestCaseSize = 1000;

constexpr uint32_t GenMask(int size) {
  uint32_t mask = 0;
  for (int i = 0; i < size; ++i) {
    mask = (mask << 1) | 1;
  }
  return mask;
}

uint32_t GetRandom32() {
  static bool init = false;
  static std::random_device rd;
  static std::mt19937 gen;
  static std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);
  if (!init) {
    gen = std::mt19937(rd());
    init = true;
  }
  return dis(gen);
}

bool CheckPte(uint32_t value) {
  Pte pte(value);
  bool result = true;
  uint32_t d = (value >> 7) & 1;
  uint32_t a = (value >> 6) & 1;
  uint32_t g = (value >> 5) & 1;
  uint32_t u = (value >> 4) & 1;
  uint32_t x = (value >> 3) & 1;
  uint32_t w = (value >> 2) & 1;
  uint32_t r = (value >> 1) & 1;
  uint32_t v = value & 1;
  uint32_t rsw = ((value >> 8) & GenMask(2));
  uint32_t ppn0 = ((value >> 10) & GenMask(10));
  uint32_t ppn1 = ((value >> 20) & GenMask(12));
  uint32_t ppn = ((value >> 10) & GenMask(22));
  result &= pte.GetD() == d;
  result &= pte.GetA() == a;
  result &= pte.GetG() == g;
  result &= pte.GetU() == u;
  result &= pte.GetX() == x;
  result &= pte.GetW() == w;
  result &= pte.GetR() == r;
  result &= pte.GetV() == v;
  result &= pte.GetRsw() == rsw;
  result &= pte.GetPpn0() == ppn0;
  result &= pte.GetPpn1() == ppn1;
  result &= pte.GetPpn() == ppn;
  if (!result) {
    std::cout << "pte = " << std::bitset<32>(value) << std::endl;
    std::cout << "GetD = " << pte.GetD() << " vs D = " << d << std::endl;
    std::cout << "GetA = " << pte.GetA() << " vs A = " << a << std::endl;
    std::cout << "GetG = " << pte.GetG() << " vs G = " << g << std::endl;
    std::cout << "GetU = " << pte.GetU() << " vs U = " << u << std::endl;
    std::cout << "GetX = " << pte.GetX() << " vs X = " << x << std::endl;
    std::cout << "GetW = " << pte.GetW() << " vs W = " << w << std::endl;
    std::cout << "GetR = " << pte.GetR() << " vs R = " << r << std::endl;
    std::cout << "GetV = " << pte.GetV() << " vs V = " << v << std::endl;
    std::cout << "GetRsw = " << std::bitset<2>(pte.GetRsw()) << " vs RSW = " << std::bitset<2>(rsw) << std::endl;
    std::cout << "GetPpn0 = " << std::bitset<10>(pte.GetPpn0()) << " vs ppn0 = " << std::bitset<10>(ppn0) << std::endl;
    std::cout << "GetPpn1 = " << std::bitset<12>(pte.GetPpn1()) << " vs ppn1 = " << std::bitset<12>(ppn1) << std::endl;
    std::cout << "GetPpn = " << std::bitset<22>(pte.GetPpn()) << " vs ppn = " << std::bitset<22>(ppn) << std::endl;
  }
  return result;
}

bool CheckValidAndLeaf() {
  Pte pte;
  bool result = true;
  for (int i = 0; i <= 0b1111; ++i) {
    int v = i & 1;
    int r = (i >> 1) & 1;
    int w = (i >> 2) & 1;
    int x = (i >> 3) & 1;
    bool invalid = (v == 0 || (r == 0 && w == 1));
    bool leaf = (r == 0 && x == 0 && w == 0);

    uint32_t pte_value = (GetRandom32() & ~0b01111) | i;
    pte = pte_value;
    result &= pte.IsValid() != invalid;
    result &= pte.IsLeaf() == leaf;
    if (!result) {
      std::cerr << "pte = " << std::bitset<32>(pte_value) << std::endl;
      std::cerr << "pte.IsValid() = " << std::boolalpha << pte.IsValid() << std::endl;
      std::cerr << "pte.IsLeaf() = " << pte.IsLeaf() << std::endl;
    }
  }
  return result;
}

bool WriteTest() {
  Pte pte;
  bool result = true;
  for (int i = 0; i < 0b1111; ++i) {
    uint32_t access_before = i & 1;
    uint32_t access_after = (i >> 1) & 1;
    uint32_t dirty_before = (i >> 2) & 1;
    uint32_t dirty_after = (i >> 2) & 1;
    uint32_t pte_value = (GetRandom32() & ~(0b11 << 6)) | (access_before << 6) | (dirty_before << 7);
    pte = pte_value;
    pte.SetA(access_after);
    pte.SetD(dirty_after);
    result &= pte.GetA() == access_after;
    result &= pte.GetD() == dirty_after;
    if (!result) {
      std::cerr << "pte = " << std::bitset<32>(pte.GetValue()) << std::endl;
      std::cerr << "pte.GetA() = " << pte.GetA() << std::endl;
      std::cerr << "pte.GetD() = " << pte.GetD() << std::endl;
    }
  }
  return result;
}

bool Test() {
  bool test_result = true;
  for (int i = 0; i < kTestCaseSize && test_result; ++i) {
    uint32_t test_case = GetRandom32();
    test_result &= CheckPte(test_case);
  }
  test_result &= CheckValidAndLeaf();
  test_result &= WriteTest();
  return test_result;
}

int main() {
  bool result = Test();
  if (result) {
    std::cout << "Pte test passed." << std::endl;
  } else {
    std::cout << "PTE test failed." << std::endl;
  }
  return result ? 0 : 1;
}

