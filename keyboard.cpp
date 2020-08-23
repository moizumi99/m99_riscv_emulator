//
// Created by moiz on 8/22/20.
//

#include <termios.h>
#include <stdio.h>
#include "keyboard.h"

namespace RISCV_EMULATOR {

// Keyboard input without echoing.
int get_key() {
  int ch = 0;
  struct termios term_old, term_new;
  tcgetattr(0, &term_old);
  term_new = term_old;
  term_new.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(0, TCSANOW, &term_new);
  ch = getchar();
  tcsetattr(0, TCSADRAIN, &term_old);
  return ch;
};

} // RISCV_EMULATOR
