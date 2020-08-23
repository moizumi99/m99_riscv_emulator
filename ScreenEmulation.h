//
// Created by moiz on 8/29/20.
//

#ifndef ASSEMBLER_TEST_SCREENEMULATION_H
#define ASSEMBLER_TEST_SCREENEMULATION_H

class ScreenEmulation {
 public:
  ScreenEmulation();
  ~ScreenEmulation();

  char getchar();
  void putchar(char c);

 private:
  void ScreenInit();
  void ScreenExit();
};

#endif  // ASSEMBLER_TEST_SCREENEMULATION_H
