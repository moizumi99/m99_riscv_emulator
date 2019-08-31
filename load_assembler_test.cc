#include "RISCV_cpu.h"
#include "load_assembler.h"
#include <iostream>
#include <string>

using namespace std;

uint32_t asm_add(uint32_t, uint32_t, uint32_t);

template <class T>
void print_binary(T value) {
  int bitwidth = sizeof(T) * 8;
  for (int i = 0; i < bitwidth; i++) {
    printf("%d", (value >> (bitwidth - i - 1)) & 1);
    if (i % 8 == 7) {
      printf(" ");
    }
  }
}

template <class T>
bool check_code(string text, T cmd, T exp, bool verbose=false) {
  bool error = false;
  if (verbose) {
    cout << text;
    printf(": %04X (", cmd);
    print_binary(cmd);
    printf(")");
  }
  if (cmd == exp) {
    printf(" - Pass\n");
  } else {
    printf(" - Error (");
    print_binary(exp);
    printf(")\n");
    error = true;
  }
  return error;
}


bool test_asm_add(bool verbose=false) {
  bool error = false;
  uint32_t cmd, exp;
  cmd = asm_add(T0, T1, T2);
  exp = 0b00000000011100110000001010110011;
  error |= check_code("ADD T0, T1, T2", cmd, exp, verbose);

  cmd = asm_add(T3, T2, T1);
  exp = 0b00000000011000111000111000110011;
  error |= check_code("ADD T3, T1, T2", cmd, exp, verbose);

  if (error) {
    printf("asm_add test failed\n");
  }
  return error;
}

int main() {
  bool error = false;
  error |= test_asm_add(true);

  if (error) {
    printf("Test failed\n");
  } else {
    printf("Test passed\n");
  }
  return error;
}