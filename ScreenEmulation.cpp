//
// Created by moiz on 8/29/20.
//

#include <ncurses.h>
#include "ScreenEmulation.h"

ScreenEmulation::ScreenEmulation() {
  ScreenInit();
}

ScreenEmulation::~ScreenEmulation() {
  ScreenExit();
}

void ScreenEmulation::ScreenInit() {
  // ncurse initialization.
  initscr();
  // Capture key stroke without storing them to buffer.
  // cbreak();
  raw();  // raw() will make Ctrl-C captured. Test enough before using this.
  noecho();
  // Capture special keys.
  keypad(stdscr, TRUE);
  // Allow scrolling.
  scrollok(stdscr, TRUE);
  // Don't wait for key hit.
  nodelay(stdscr, TRUE);
}

void ScreenEmulation::ScreenExit() {
  endwin();
}

bool ScreenEmulation::CheckInput() {
  if (++counter_ < kThreshold) {
    return key_valid_;
  } else {
    counter_ = 0;
  }
  if (key_valid_) {
    return key_valid_;
  }
  int c = getch();
  if (c == ERR) {
    return key_valid_;
  }
  key_value_ = c;
  key_valid_ = true;
  return true;
}

int ScreenEmulation::GetKeyValue() {
  key_valid_ = false;
  return key_value_;
}

void ScreenEmulation::putchar(int c) {
  addch(c);
  wrefresh(stdscr);
}
