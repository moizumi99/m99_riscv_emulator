//
// Created by moiz on 2/15/20.
//

#ifndef ASSEMBLER_TEST_PTE_H
#define ASSEMBLER_TEST_PTE_H

#include <cstdint>

class Pte {
public:
  Pte();
  Pte(uint32_t pte_value);
  Pte& operator=(uint32_t pte_value);
  uint32_t GetValue() const;
  uint32_t GetPpn() const;
  uint32_t GetPpn1() const;
  uint32_t GetPpn0() const;
  uint32_t GetRsw() const;
  uint32_t GetD() const;
  uint32_t GetA() const;
  uint32_t GetG() const;
  uint32_t GetU() const;
  uint32_t GetX() const;
  uint32_t GetW() const;
  uint32_t GetR() const;
  uint32_t GetV() const;
  void SetA(int value);
  void SetD(int value);
  bool IsLeaf() const;
  bool IsValid() const;
private:
  uint32_t pte_;
};

#endif //ASSEMBLER_TEST_PTE_H
