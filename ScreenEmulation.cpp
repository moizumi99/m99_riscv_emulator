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
  cbreak();
  //raw();  // raw() will make Ctrl-C captured. Test enough before using this.
  noecho();
  // Capture special keys.
  keypad(stdscr, TRUE);
  // Allow scrolling.
  scrollok(stdscr, TRUE);
  // Don't wait for return.
  nodelay(stdscr, TRUE);
}

void ScreenEmulation::ScreenExit() {
  endwin();
}

char ScreenEmulation::getchar() {
  return getch();
}

void ScreenEmulation::putchar(char c) {
  addch(c);
  wrefresh(stdscr);
}
