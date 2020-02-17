//
// Created by moiz on 2/15/20.
//

#include "pte.h"
#include "bit_tools.h"

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

void Pte::SetA(int value) {
  if (value) {
    pte_ |= 1 << 6;
  } else {
    pte_ &= ~(1 << 6);
  }
}

void Pte::SetD(int value) {
  if (value) {
    pte_ |= 1 << 7;
  } else {
    pte_ &= ~(1 << 7);
  }
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
  return GetR() == 0 && GetW() == 0 && GetX() == 0;
}