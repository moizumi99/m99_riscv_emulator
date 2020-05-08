//
// Created by moiz on 2/15/20.
//

#include "pte.h"
#include "bit_tools.h"


// 32 bit version,
Pte32::Pte32() : Pte32(0) {}

Pte32::Pte32(uint32_t pte_value) : pte_(pte_value) {}

Pte32& Pte32::operator=(uint32_t pte_value) {
  this->pte_ = pte_value;
  return (*this);
}

uint32_t Pte32::GetValue() const {
  return pte_;
};

uint32_t Pte32::GetPpn() const {
  return bitcrop(pte_, 22, 10);
}

uint32_t Pte32::GetPpn1() const {
  return bitcrop(pte_, 12, 20);
}

uint32_t Pte32::GetPpn0() const {
  return bitcrop(pte_, 10, 10);
}

uint32_t Pte32::GetRsw() const {
  return bitcrop(pte_, 2, 8);
}

uint32_t Pte32::GetD() const {
  return bitcrop(pte_, 1, 7);
}
uint32_t Pte32::GetA() const {
  return bitcrop(pte_, 1, 6);
}
uint32_t Pte32::GetG() const {
  return bitcrop(pte_, 1, 5);
}
uint32_t Pte32::GetU() const {
  return bitcrop(pte_, 1, 4);
}
uint32_t Pte32::GetX() const {
  return bitcrop(pte_, 1, 3);
}
uint32_t Pte32::GetW() const {
  return bitcrop(pte_, 1, 2);
}
uint32_t Pte32::GetR() const {
  return bitcrop(pte_, 1, 1);
}
uint32_t Pte32::GetV() const {
  return bitcrop(pte_, 1, 0);
}

void Pte32::SetPpn(uint32_t ppn) {
  pte_ = (pte_ & ~GenMask<uint32_t>(22, 10)) | (ppn << 10);
}

void Pte32::SetRsw(uint32_t rsw) {
  pte_ = (pte_ & ~GenMask<uint32_t>(2, 8)) | (rsw << 8);
}


void Pte32::SetD(int value) {
  pte_ = SetBit(pte_, value, 7);
}

void Pte32::SetA(int value) {
  pte_ = SetBit(pte_, value, 6);
}

void Pte32::SetG(int value) {
  pte_ = SetBit(pte_, value, 5);
}

void Pte32::SetU(int value) {
  pte_ = SetBit(pte_, value, 4);
}

void Pte32::SetX(int value) {
  pte_ = SetBit(pte_, value, 3);
}
void Pte32::SetW(int value) {
  pte_ = SetBit(pte_, value, 2);
}
void Pte32::SetR(int value) {
  pte_ = SetBit(pte_, value, 1);
}

void Pte32::SetV(int value) {
  pte_ = SetBit(pte_, value, 0);
}

bool Pte32::IsValid() const {
  uint32_t v = this->GetV();
  uint32_t r = this->GetR();
  uint16_t w = this->GetW();
  if (v == 0 || (r == 0 && w == 1)) {
    return false;
  } else {
    return true;
  }
}

bool Pte32::IsLeaf() const {
  return GetR() != 0 || GetW() != 0 || GetX() != 0;
}

// 64 bit version,
Pte64::Pte64() : pte_(0) {}

Pte64::Pte64(uint64_t pte_value) : pte_(pte_value) {}

Pte64& Pte64::operator=(uint64_t pte_value) {
  this->pte_ = pte_value;
  return (*this);
}

uint64_t Pte64::GetValue() const {
  return pte_;
};

uint32_t Pte64::GetPpn() const {
  return bitcrop(pte_, 44, 10);
}

uint32_t Pte64::GetPpn2() const {
  return bitcrop(pte_, 26, 28);
}

uint32_t Pte64::GetPpn1() const {
  return bitcrop(pte_, 9, 19);
}

uint32_t Pte64::GetPpn0() const {
  return bitcrop(pte_, 9, 10);
}

uint32_t Pte64::GetRsw() const {
  return bitcrop(pte_, 2, 8);
}

uint32_t Pte64::GetD() const {
  return bitcrop(pte_, 1, 7);
}
uint32_t Pte64::GetA() const {
  return bitcrop(pte_, 1, 6);
}
uint32_t Pte64::GetG() const {
  return bitcrop(pte_, 1, 5);
}
uint32_t Pte64::GetU() const {
  return bitcrop(pte_, 1, 4);
}
uint32_t Pte64::GetX() const {
  return bitcrop(pte_, 1, 3);
}
uint32_t Pte64::GetW() const {
  return bitcrop(pte_, 1, 2);
}
uint32_t Pte64::GetR() const {
  return bitcrop(pte_, 1, 1);
}
uint32_t Pte64::GetV() const {
  return bitcrop(pte_, 1, 0);
}

void Pte64::SetPpn(uint64_t ppn) {
  pte_ = (pte_ & ~GenMask<uint64_t>(22, 10)) | (ppn << 10);
}

void Pte64::SetRsw(uint32_t rsw) {
  pte_ = (pte_ & ~GenMask<uint64_t>(2, 8)) | (rsw << 8);
}

void Pte64::SetD(int value) {
  pte_ = SetBit(pte_, value, 7);
}

void Pte64::SetA(int value) {
  pte_ = SetBit(pte_, value, 6);
}

void Pte64::SetG(int value) {
  pte_ = SetBit(pte_, value, 5);
}

void Pte64::SetU(int value) {
  pte_ = SetBit(pte_, value, 4);
}

void Pte64::SetX(int value) {
  pte_ = SetBit(pte_, value, 3);
}
void Pte64::SetW(int value) {
  pte_ = SetBit(pte_, value, 2);
}
void Pte64::SetR(int value) {
  pte_ = SetBit(pte_, value, 1);
}

void Pte64::SetV(int value) {
  pte_ = SetBit(pte_, value, 0);
}

bool Pte64::IsValid() const {
  uint32_t v = this->GetV();
  uint32_t r = this->GetR();
  uint16_t w = this->GetW();
  if (v == 0 || (r == 0 && w == 1)) {
    return false;
  } else {
    return true;
  }
}

bool Pte64::IsLeaf() const {
  return GetR() != 0 || GetW() != 0 || GetX() != 0;
}