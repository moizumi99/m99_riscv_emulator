//
// Created by moiz on 2/15/20.
//

#include "pte.h"
#include "bit_tools.h"

constexpr uint32_t GenMask(int size, int shift) {
  uint32_t mask = 0;
  for (int i = 0; i < size; ++i) {
    mask = (mask << 1) | 1;
  }
  return mask << shift;
}

Pte::Pte() : Pte(0) {}

Pte::Pte(uint32_t pte_value) : pte_(pte_value) {}

Pte& Pte::operator=(uint32_t pte_value) {
  this->pte_ = pte_value;
}

uint32_t Pte::GetValue() const {
  return pte_;
};

uint32_t Pte::GetPpn() const {
  return bitcrop(pte_, 22, 10);
}

uint32_t Pte::GetPpn1() const {
  return bitcrop(pte_, 12, 20);
}

uint32_t Pte::GetPpn0() const {
  return bitcrop(pte_, 10, 10);
}

uint32_t Pte::GetRsw() const {
  return bitcrop(pte_, 2, 8);
}

uint32_t Pte::GetD() const {
  return bitcrop(pte_, 1, 7);
}
uint32_t Pte::GetA() const {
  return bitcrop(pte_, 1, 6);
}
uint32_t Pte::GetG() const {
  return bitcrop(pte_, 1, 5);
}
uint32_t Pte::GetU() const {
  return bitcrop(pte_, 1, 4);
}
uint32_t Pte::GetX() const {
  return bitcrop(pte_, 1, 3);
}
uint32_t Pte::GetW() const {
  return bitcrop(pte_, 1, 2);
}
uint32_t Pte::GetR() const {
  return bitcrop(pte_, 1, 1);
}
uint32_t Pte::GetV() const {
  return bitcrop(pte_, 1, 0);
}

void Pte::SetPpn(uint32_t ppn) {
  pte_ = (pte_ & ~GenMask(22, 10)) | (ppn << 10);
}

void Pte::SetRsw(uint32_t rsw) {
  pte_ = (pte_ & ~GenMask(2, 8)) | (rsw << 8);
}

uint32_t SetBit(uint32_t data, int value, int pos) {
  if (value) {
    return data | (1 << pos);
  } else {
    return data & ~(1 << pos);
  }
}

void Pte::SetD(int value) {
  pte_ = SetBit(pte_, value, 7);
}

void Pte::SetA(int value) {
  pte_ = SetBit(pte_, value, 6);
}

void Pte::SetG(int value) {
  pte_ = SetBit(pte_, value, 5);
}

void Pte::SetU(int value) {
  pte_ = SetBit(pte_, value, 4);
}

void Pte::SetX(int value) {
  pte_ = SetBit(pte_, value, 3);
}
void Pte::SetW(int value) {
  pte_ = SetBit(pte_, value, 2);
}
void Pte::SetR(int value) {
  pte_ = SetBit(pte_, value, 1);
}

void Pte::SetV(int value) {
  pte_ = SetBit(pte_, value, 0);
}

bool Pte::IsValid() const {
  uint32_t v = this->GetV();
  uint32_t r = this->GetR();
  uint16_t w = this->GetW();
  if (v == 0 || (r == 0 && w == 1)) {
    return false;
  } else {
    return true;
  }
}

bool Pte::IsLeaf() const {
  return GetR() != 0 || GetW() != 0 || GetX() != 0;
}