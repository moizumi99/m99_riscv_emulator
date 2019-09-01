#include "RISCV_cpu.h"
#include "load_assembler.h"
#include <iostream>
#include <string>

using namespace std;

uint32_t asm_add(uint32_t rd, uint32_t rs1, uint32_t rs2);
uint32_t asm_addi(uint32_t rd, uint32_t rs1, uint32_t imm12);
uint32_t asm_sub(uint32_t rd, uint32_t rs1, uint32_t rs2);
uint32_t asm_beq(uint32_t rs1, uint32_t rs2, uint32_t offset13);
uint32_t asm_jal(uint32_t rd, uint32_t offset21);
uint32_t asm_ld(uint32_t rd, uint32_t rs1, uint32_t offset12);
uint32_t asm_sw(uint32_t rs1, uint32_t rs2, uint32_t offset12);

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
  error |= check_code("add T0, T1, T2", cmd, exp, verbose);

  cmd = asm_add(T3, T2, T1);
  exp = 0b00000000011000111000111000110011;
  error |= check_code("add T3, T1, T2", cmd, exp, verbose);

  return error;
}

bool test_asm_addi(bool verbose=false) {
  bool error = false;
  uint32_t cmd, exp;
  cmd = asm_addi(T0, T0, 0);
  exp = 0b00000000000000101000001010010011;
  error |= check_code("addi T0, T0, 0", cmd, exp, verbose);

  cmd = asm_addi(T3, T4, 0b101010101010);
  exp = 0b10101010101011101000111000010011;
  error |= check_code("addi T3, T4, 2730", cmd, exp, verbose);

  return error;
}

bool test_asm_sub(bool verbose=false) {
  bool error = false;
  uint32_t cmd, exp;
  cmd = asm_sub(T0, T1, T2);
  exp = 0b01000000011100110000001010110011;
  error |= check_code("sub T0, T0, T0", cmd, exp, verbose);

  cmd = asm_sub(T3, T2, T1);
  exp = 0b01000000011000111000111000110011;
  error |= check_code("sub T3, T1, T2", cmd, exp, verbose);

  return error;
}

bool test_asm_beq(bool verbose=false) {
  bool error = false;
  uint32_t cmd, exp;
  cmd = asm_beq(T0, T0, 0);
  exp = 0b00000000010100101000000001100011;
  error |= check_code("beq T0, T0, 0", cmd, exp, verbose);

  cmd = asm_beq(T0, T0, 0b1010101010101);
  exp = 0b11010100010100101000101001100011;
  error |= check_code("beq T3, T4, 2730", cmd, exp, verbose);

  cmd = asm_beq(T3, T4, 0b1010101010101);
  exp = 0b11010101110111100000101001100011;
  error |= check_code("beq T3, T4, 2730", cmd, exp, verbose);

  return error;
}

bool test_asm_jal(bool verbose=false) {
  bool error = false;
  uint32_t cmd, exp;
  cmd = asm_jal(T0, 0);
  exp = 0b00000000000000000000001011101111;
  error |= check_code("jal T0, 0", cmd, exp, verbose);

  cmd = asm_jal(T4, 0b101010101010101010101);
  exp = 0b11010101010001010101111011101111;
  error |= check_code("jal T4, 1398101", cmd, exp, verbose);

  cmd = asm_jal(ZERO, 0);
  exp = 0b00000000000000000000000001101111;
  error |= check_code("jal ZERO, 0", cmd, exp, verbose);

  return error;
}

bool test_asm_ld(bool verbose=false) {
  bool error = false;
  uint32_t cmd, exp;
  cmd = asm_ld(T0, T0, 0);
  exp = 0b00000000000000101011001010000011;
  error |= check_code("ld T0, T0, 0", cmd, exp, verbose);

  cmd = asm_ld(T3, T4, 0);
  exp = 0b00000000000011101011111000000011;
  error |= check_code("ld T3, T4, 0", cmd, exp, verbose);

  cmd = asm_ld(T3, T4, 2730);
  exp = 0b10101010101011101011111000000011;
  error |= check_code("ld T3, T4, 2730", cmd, exp, verbose);

  return error;
}

bool test_asm_sw(bool verbose=false) {
  bool error = false;
  uint32_t cmd, exp;
  cmd = asm_sw(T0, T0, 0);
  exp = 0b00000000010100101010000000100011;
  error |= check_code("sw T0, T0, 0", cmd, exp, verbose);
  return error;
}

int main() {
  bool verbose = true;
  bool error = false;
  error |= test_asm_add(verbose);
  error |= test_asm_addi(verbose);
  error |= test_asm_sub(verbose);
  error |= test_asm_beq(verbose);
  error |= test_asm_jal(verbose);
  error |= test_asm_ld(verbose);

  if (error) {
    printf("Test failed\n");
  } else {
    printf("Test passed\n");
  }
  return error;
}