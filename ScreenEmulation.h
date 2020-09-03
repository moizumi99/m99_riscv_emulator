//
// Created by moiz on 8/29/20.
//

#ifndef ASSEMBLER_TEST_SCREENEMULATION_H
#define ASSEMBLER_TEST_SCREENEMULATION_H

#include "ncurses.h"

class ScreenEmulation {
 public:
  ScreenEmulation();
  ~ScreenEmulation();

  bool CheckInput();
  int GetKeyValue();
  void putchar(int c);

  int counter_ = 0;
  static constexpr int kThreshold = 1000;

 private:
  int key_value_;
  bool key_valid_ = false;
  void ScreenInit();
  void ScreenExit();
};

#endif  // ASSEMBLER_TEST_SCREENEMULATION_H
