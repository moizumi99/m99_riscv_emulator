#include "RISCV_cpu.h"
#include "instruction_encdec.h"
#include "load_assembler.h"
#include <iostream>
#include <string>

using namespace std;

template <class T> void print_binary(T value) {
  int bitwidth = sizeof(T) * 8;
  for (int i = 0; i < bitwidth; i++) {
    printf("%d", (value >> (bitwidth - i - 1)) & 1);
    if (i % 8 == 7) {
      printf(" ");
    }
  }
}

template <class T>
bool check_code(string text, T cmd, T exp, bool verbose = false) {
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

bool check_equal(string text, uint32_t value, uint32_t exp,
                 bool verbose = false) {
  bool error = value != exp;
  if (verbose) {
    cout << text;
    printf(": %d (", value);
    print_binary(value);
    printf(")");
    if (!error) {
      printf(" - Pass\n");
    } else {
      printf(" - Error (");
      print_binary(exp);
      printf(")\n");
    }
  }
  return error;
}

bool test_asm_add(bool verbose = false) {
  bool subtest_verbose = false;

  bool error = false;
  uint32_t cmd, exp;
  cmd = asm_add(T0, T1, T2);
  exp = 0b00000000011100110000001010110011;
  error |= check_code("add T0, T1, T2", cmd, exp, verbose);
  r_type add;
  add.set_value(exp);
  error |= check_equal("add", add.opcode, OPCODE_ADD, subtest_verbose);
  error |= check_equal("add7", add.funct7, FUNC_ADD);
  error |= check_equal("add3", add.funct3, FUNC3_ADD);
  error |= check_equal("rd", add.rd, T0, subtest_verbose);
  error |= check_equal("rs1", add.rs1, T1, subtest_verbose);
  error |= check_equal("rs2", add.rs2, T2, subtest_verbose);

  cmd = asm_add(T1, T1, T0);
  exp = 0b00000000010100110000001100110011;
  error |= check_code("add T1, T1, T0", cmd, exp, verbose);
  add.set_value(exp);
  error |= check_equal("add", add.opcode, OPCODE_ADD, subtest_verbose);
  error |= check_equal("add7", add.funct7, FUNC_ADD);
  error |= check_equal("add3", add.funct3, FUNC3_ADD);
  error |= check_equal("rd", add.rd, T1, subtest_verbose);
  error |= check_equal("rs1", add.rs1, T1, subtest_verbose);
  error |= check_equal("rs2", add.rs2, T0, subtest_verbose);

  cmd = asm_add(T3, T2, T1);
  exp = 0b00000000011000111000111000110011;
  error |= check_code("add T3, T1, T2", cmd, exp, verbose);
  add.set_value(exp);
  error |= check_equal("add", add.opcode, OPCODE_ADD, subtest_verbose);
  error |= check_equal("add7", add.funct7, FUNC_ADD);
  error |= check_equal("add3", add.funct3, FUNC3_ADD);
  error |= check_equal("rd", add.rd, T3, subtest_verbose);
  error |= check_equal("rs1", add.rs1, T2, subtest_verbose);
  error |= check_equal("rs2", add.rs2, T1, subtest_verbose);

  if (verbose && error) {
    printf("Add test failed\n");
  }
  return error;
}

bool test_asm_addi(bool verbose = false) {
  bool error = false;
  bool subtest_verbose = false;
  uint32_t cmd, exp;
  cmd = asm_addi(T0, T0, 0);
  exp = 0b00000000000000101000001010010011;
  error |= check_code("addi T0, T0, 0", cmd, exp, verbose);
  i_type addi;
  addi.set_value(exp);
  error |= check_equal("addi", addi.opcode, OPCODE_ADDI, subtest_verbose);
  error |= check_equal("addi3", addi.funct3, FUNC3_ADDI);
  error |= check_equal("rd", addi.rd, T0, subtest_verbose);
  error |= check_equal("rs1", addi.rs1, T0, subtest_verbose);
  error |= check_equal("imm12(0)", addi.imm12, 0, subtest_verbose);

  cmd = asm_addi(T3, T4, 0b101010101010);
  exp = 0b10101010101011101000111000010011;
  error |= check_code("addi T3, T4, -1366", cmd, exp, verbose);
  addi.set_value(exp);
  error |= check_equal("addi", addi.opcode, OPCODE_ADDI, subtest_verbose);
  error |= check_equal("addi3", addi.funct3, FUNC3_ADDI);
  error |= check_equal("rd", addi.rd, T3, subtest_verbose);
  error |= check_equal("rs1", addi.rs1, T4, subtest_verbose);
  error |= check_equal("imm12(-1366)", addi.imm12, -1366, subtest_verbose);

  if (verbose && error) {
    printf("Addi test failed\n");
  }
  return error;
}

bool test_asm_sub(bool verbose = false) {
  bool subtest_verbose = false;

  bool error = false;
  uint32_t cmd, exp;
  cmd = asm_sub(T0, T1, T2);
  exp = 0b01000000011100110000001010110011;
  error |= check_code("sub T0, T1, T2", cmd, exp, verbose);
  r_type sub;
  sub.set_value(exp);
  error |= check_equal("sub", sub.opcode, OPCODE_ADD, subtest_verbose);
  error |= check_equal("sub7", sub.funct7, FUNC_SUB);
  error |= check_equal("sub3", sub.funct3, FUNC3_SUB);
  error |= check_equal("rd", sub.rd, T0, subtest_verbose);
  error |= check_equal("rs1", sub.rs1, T1, subtest_verbose);
  error |= check_equal("rs2", sub.rs2, T2, subtest_verbose);

  cmd = asm_sub(T3, T2, T1);
  exp = 0b01000000011000111000111000110011;
  error |= check_code("sub T3, T1, T2", cmd, exp, verbose);
  sub.set_value(exp);
  error |= check_equal("sub", sub.opcode, OPCODE_ADD, subtest_verbose);
  error |= check_equal("sub7", sub.funct7, FUNC_SUB);
  error |= check_equal("sub3", sub.funct3, FUNC3_SUB);
  error |= check_equal("rd", sub.rd, T3, subtest_verbose);
  error |= check_equal("rs1", sub.rs1, T2, subtest_verbose);
  error |= check_equal("rs2", sub.rs2, T1, subtest_verbose);

  if (verbose && error) {
    printf("SUB test failed\n");
  }
  return error;
}

bool test_asm_beq(bool verbose = false) {
  bool subtest_verbose = false;
  bool error = false;
  uint32_t cmd, exp;
  cmd = asm_beq(T0, T0, 0);
  exp = 0b00000000010100101000000001100011;
  error |= check_code("beq T0, T0, 0", cmd, exp, verbose);
  b_type beq;
  beq.set_value(exp);
  error |= check_equal("beq", beq.opcode, OPCODE_B, subtest_verbose);
  error |= check_equal("beq3", beq.funct3, FUNC3_BEQ);
  error |= check_equal("imm13", beq.imm13, 0, subtest_verbose);
  error |= check_equal("rs1", beq.rs1, T0, subtest_verbose);
  error |= check_equal("rs2", beq.rs2, T0, subtest_verbose);

  cmd = asm_beq(T0, T0, 0b1010101010101);
  exp = 0b11010100010100101000101001100011;
  error |= check_code("beq T0, T0, -1366", cmd, exp, verbose);
  beq.set_value(exp);
  error |= check_equal("beq", beq.opcode, OPCODE_B, subtest_verbose);
  error |= check_equal("beq3", beq.funct3, FUNC3_BEQ);
  error |= check_equal("imm13", beq.imm13, -2732, subtest_verbose);
  error |= check_equal("rs1", beq.rs1, T0, subtest_verbose);
  error |= check_equal("rs2", beq.rs2, T0, subtest_verbose);

  cmd = asm_beq(T3, T4, 0b1010101010101);
  exp = 0b11010101110111100000101001100011;
  beq.set_value(exp);
  error |= check_code("beq T3, T4, -2732", cmd, exp, verbose);
  error |= check_equal("beq", beq.opcode, OPCODE_B, subtest_verbose);
  error |= check_equal("beq3", beq.funct3, FUNC3_BEQ);
  error |= check_equal("imm13", beq.imm13, -2732, subtest_verbose);
  error |= check_equal("rs1", beq.rs1, T3, subtest_verbose);
  error |= check_equal("rs2", beq.rs2, T4, subtest_verbose);

  if (verbose && error) {
    printf("BEQ test failed\n");
  }
  return error;
}

bool test_asm_jal(bool verbose = false) {
  bool subtest_verbose = false;
  bool error = false;
  uint32_t cmd, exp;
  cmd = asm_jal(T0, 0);
  exp = 0b00000000000000000000001011101111;
  error |= check_code("jal T0, 0", cmd, exp, verbose);
  j_type inst;
  inst.set_value(exp);
  error |= check_equal("jal", inst.opcode, OPCODE_J, subtest_verbose);
  error |= check_equal("imm21", inst.imm21, 0, subtest_verbose);
  error |= check_equal("rd", inst.rd, T0, subtest_verbose);

  cmd = asm_jal(T4, 0b101010101010101010101);
  exp = 0b11010101010001010101111011101111;
  error |= check_code("jal T4, 1398101", cmd, exp, verbose);
  inst.set_value(exp);
  error |= check_equal("jal", inst.opcode, OPCODE_J, subtest_verbose);
  error |= check_equal("imm21", inst.imm21, -699052, subtest_verbose);
  error |= check_equal("rd", inst.rd, T4, subtest_verbose);

  cmd = asm_jal(ZERO, 0);
  exp = 0b00000000000000000000000001101111;
  error |= check_code("jal ZERO, 0", cmd, exp, verbose);
  inst.set_value(exp);
  error |= check_equal("jal", inst.opcode, OPCODE_J, subtest_verbose);
  error |= check_equal("imm21", inst.imm21, 0, subtest_verbose);
  error |= check_equal("rd", inst.rd, ZERO, subtest_verbose);

  cmd = asm_jal(ZERO, -16);
  exp = 0b11111111000111111111000001101111;
  error |= check_code("jal ZERO, -16", cmd, exp, verbose);
  inst.set_value(exp);
  error |= check_equal("jal", inst.opcode, OPCODE_J, subtest_verbose);
  error |= check_equal("imm21", inst.imm21, -16, subtest_verbose);
  error |= check_equal("rd", inst.rd, ZERO, subtest_verbose);

  if (verbose && error) {
    printf("BEQ test failed\n");
  }
  return error;
}

bool test_asm_ld(bool verbose = false) {
  bool subtest_verbose = false;
  bool error = false;
  uint32_t cmd, exp;
  cmd = asm_ld(T0, T0, 0);
  exp = 0b00000000000000101011001010000011;
  error |= check_code("ld T0, T0, 0", cmd, exp, verbose);
  i_type inst;
  inst.set_value(exp);
  error |= check_equal("ld", inst.opcode, OPCODE_LD, subtest_verbose);
  error |= check_equal("ld3", inst.funct3, FUNC3_LD);
  error |= check_equal("rd", inst.rd, T0, subtest_verbose);
  error |= check_equal("rs1", inst.rs1, T0, subtest_verbose);
  error |= check_equal("imm12(0)", inst.imm12, 0, subtest_verbose);

  cmd = asm_ld(T3, T4, 0);
  exp = 0b00000000000011101011111000000011;
  error |= check_code("ld T3, T4, 0", cmd, exp, verbose);
  inst.set_value(exp);
  error |= check_equal("ld", inst.opcode, OPCODE_LD, subtest_verbose);
  error |= check_equal("ld3", inst.funct3, FUNC3_LD);
  error |= check_equal("rd", inst.rd, T3, subtest_verbose);
  error |= check_equal("rs1", inst.rs1, T4, subtest_verbose);
  error |= check_equal("imm12(0)", inst.imm12, 0, subtest_verbose);

  cmd = asm_ld(T3, T4, 2730);
  exp = 0b10101010101011101011111000000011;
  error |= check_code("ld T3, T4, -1366", cmd, exp, verbose);
  inst.set_value(exp);
  error |= check_equal("ld", inst.opcode, OPCODE_LD, subtest_verbose);
  error |= check_equal("ld3", inst.funct3, FUNC3_LD);
  error |= check_equal("rd", inst.rd, T3, subtest_verbose);
  error |= check_equal("rs1", inst.rs1, T4, subtest_verbose);
  error |= check_equal("imm12(0)", inst.imm12, -1366, subtest_verbose);

  if (verbose && error) {
    printf("LD test failed\n");
  }
  return error;
}

bool test_asm_sw(bool verbose = false) {
  bool subtest_verbose = false;
  bool error = false;
  uint32_t cmd, exp;
  cmd = asm_sw(T0, T0, 0);
  exp = 0b00000000010100101010000000100011;
  error |= check_code("sw T0, T0, 0", cmd, exp, verbose);

  s_type inst;
  inst.set_value(exp);
  error |= check_equal("sw", inst.opcode, OPCODE_S, subtest_verbose);
  error |= check_equal("sw3", inst.funct3, FUNC3_SW);
  error |= check_equal("rs1", inst.rd, T0, subtest_verbose);
  error |= check_equal("rs2", inst.rs1, T0, subtest_verbose);
  error |= check_equal("imm12(0)", inst.imm12, 0, subtest_verbose);

  return error;
}

bool test_asm_jalr(bool verbose = false) {
  bool subtest_verbose = false;
  bool error = false;
  uint32_t cmd, exp;
  cmd = asm_jalr(T0, T0, 0);
  exp = 0b00000000000000101000001011100111;
  error |= check_code("jalr T0, T0, 0", cmd, exp, verbose);
  i_type inst;
  inst.set_value(exp);
  error |= check_equal("jalr", inst.opcode, OPCODE_JALR, subtest_verbose);
  error |= check_equal("jalr3", inst.funct3, FUNC3_JALR);
  error |= check_equal("rd", inst.rd, T0, subtest_verbose);
  error |= check_equal("rs1", inst.rs1, T0, subtest_verbose);
  error |= check_equal("imm12(0)", inst.imm12, 0, subtest_verbose);

  cmd = asm_jalr(T3, T4, 0);
  exp = 0b00000000000011101000111001100111;
  error |= check_code("jalr T3, T4, 0", cmd, exp, verbose);
  inst.set_value(exp);
  error |= check_equal("jalr", inst.opcode, OPCODE_JALR, subtest_verbose);
  error |= check_equal("jalr3", inst.funct3, FUNC3_JALR);
  error |= check_equal("rd", inst.rd, T3, subtest_verbose);
  error |= check_equal("rs1", inst.rs1, T4, subtest_verbose);
  error |= check_equal("imm12(0)", inst.imm12, 0, subtest_verbose);

  cmd = asm_jalr(T3, T4, 2730);
  exp = 0b10101010101011101000111001100111;
  error |= check_code("jalr T3, T4, 2730", cmd, exp, verbose);
  inst.set_value(exp);
  error |= check_equal("jalr", inst.opcode, OPCODE_JALR, subtest_verbose);
  error |= check_equal("jalr3", inst.funct3, FUNC3_JALR);
  error |= check_equal("rd", inst.rd, T3, subtest_verbose);
  error |= check_equal("rs1", inst.rs1, T4, subtest_verbose);
  error |= check_equal("imm12(0)", inst.imm12, -1366, subtest_verbose);

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
  error |= test_asm_jalr(verbose);

  if (error) {
    printf("Test failed\n");
  } else {
    printf("Test passed\n");
  }
  return error;
}