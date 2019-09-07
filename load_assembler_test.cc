#include "RISCV_cpu.h"
#include "instruction_encdec.h"
#include "load_assembler.h"
#include <iostream>
#include <string>

using namespace std;

constexpr int TEST_NUM = 100;

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
    if (cmd == exp) {
      printf(" - Pass\n");
    } else {
      printf(" - Error (");
      print_binary(exp);
      printf(")\n");
      error = true;
    }
  }
  return error;
}

template <class T>
bool check_code_quiet(string text, T cmd, T exp, bool verbose = false) {
  bool error = check_code(text, cmd, exp, false);
  if (error & verbose) {
    error = check_code(text, cmd, exp, true);
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

bool test_r_type_decode(uint32_t instruction, uint8_t opcode, uint8_t funct3,
                        uint8_t funct7, uint8_t rd, uint8_t rs1, uint8_t rs2,
                        bool verbose = false) {
  bool error = false;
  r_type cmd;
  cmd.set_value(instruction);
  error |= check_equal("opcode", cmd.opcode, opcode, verbose);
  error |= check_equal("funct7", cmd.funct7, funct7, verbose);
  error |= check_equal("funct3", cmd.funct3, funct3, verbose);
  error |= check_equal("rd", cmd.rd, rd, verbose);
  error |= check_equal("rs1", cmd.rs1, rs1, verbose);
  error |= check_equal("rs2", cmd.rs2, rs2, verbose);
  return error;
}

bool test_r_type_decode_quiet(uint32_t instruction, uint8_t opcode,
                              uint8_t funct3, uint8_t funct7, uint8_t rd,
                              uint8_t rs1, uint8_t rs2, bool verbose = false) {
  bool error = test_r_type_decode(instruction, opcode, funct3, funct7, rd, rs1,
                                  rs2, false);
  if (error) {
    error = test_r_type_decode(instruction, opcode, funct3, funct7, rd, rs1,
                               rs2, true);
  }
  return false;
}

bool test_i_type_decode(uint32_t instruction, uint8_t opcode, uint8_t funct3,
                        uint8_t rd, uint8_t rs1, int16_t imm12,
                        bool verbose = false) {
  bool error = false;
  i_type cmd;
  cmd.set_value(instruction);
  error |= check_equal("cmd", cmd.opcode, opcode, verbose);
  error |= check_equal("funct3", cmd.funct3, funct3);
  error |= check_equal("rd", cmd.rd, rd, verbose);
  error |= check_equal("rs1", cmd.rs1, rs1, verbose);
  error |= check_equal("imm12", cmd.imm12, imm12, verbose);
  return error;
}

bool test_b_type_decode(uint32_t instruction, uint8_t opcode, uint8_t funct3,
                        uint8_t rs1, uint8_t rs2, int16_t imm13,
                        bool verbose = false) {
  bool error = false;
  b_type cmd;
  cmd.set_value(instruction);
  error |= check_equal("cmd", cmd.opcode, opcode, verbose);
  error |= check_equal("funct3", cmd.funct3, funct3);
  error |= check_equal("rs1", cmd.rs1, rs1, verbose);
  error |= check_equal("rs2", cmd.rs2, rs2, verbose);
  error |= check_equal("imm13", cmd.imm13, imm13, verbose);
  return error;
}

bool test_j_type_decode(uint32_t instruction, uint8_t opcode, uint8_t rd,
                        int32_t imm21, bool verbose = false) {
  bool error = false;
  j_type cmd;
  cmd.set_value(instruction);
  error |= check_equal("cmd", cmd.opcode, opcode, verbose);
  error |= check_equal("rd", cmd.rd, rd, verbose);
  error |= check_equal("imm21", cmd.imm21, imm21, verbose);
  return error;
}

uint32_t gen_r_type(uint32_t base, uint8_t rd, uint8_t rs1, uint8_t rs2) {
  return base | ((rd & 0x01F) << 7) | ((rs1 & 0x01F) << 15) |
         ((rs2 & 0x01F) << 20);
}

bool test_r_type(bool verbose = false) {
  enum TEST_LIST { TEST_ADD, TEST_SUB };
  bool total_error = false;

  for (int testcase : {TEST_ADD, TEST_SUB}) {
    bool error = false;
    uint32_t base;
    string cmdname;
    uint8_t opcode, funct3, funct7;
    uint32_t *funct(uint8_t, uint8_t, uint8_t);
    switch (testcase) {
    case TEST_ADD:
      base = 0b00000000000000000000000000110011;
      cmdname = "ADD";
      opcode = OPCODE_ADD;
      funct3 = FUNC3_ADD;
      funct7 = FUNC_ADD;
      break;
    case TEST_SUB:
      base = 0b01000000000000000000000000110011;
      cmdname = "SUB";
      opcode = OPCODE_ADD;
      funct3 = FUNC3_SUB;
      funct7 = FUNC_SUB;
      break;
    default:
      printf("Test case is node defined yet\n");
      return true;
      break;
    }

    for (int i = 0; i < TEST_NUM; i++) {
      uint32_t cmd;
      uint8_t rd = rand() % 32;
      uint8_t rs1 = rand() % 32;
      uint8_t rs2 = rand() % 32;
      switch (testcase) {
      case TEST_ADD:
        cmd = asm_add(rd, rs1, rs2);
        break;
      case TEST_SUB:
        cmd = asm_sub(rd, rs1, rs2);
        break;
      default:
        break;
      }
      uint32_t exp = gen_r_type(base, rd, rs1, rs2);
      string test_string = cmdname + " " + to_string(rd) + ", " +
                           to_string(rs1) + ", " + to_string(rs2);
      error |= check_code_quiet(test_string, cmd, exp, verbose);
      error |= test_r_type_decode_quiet(exp, opcode, funct3, funct7,
                                        rd, rs1, rs2, verbose);
    }
    if (verbose) {
      printf("Total %d %s random test finished. ", TEST_NUM, cmdname.c_str());
      if (error) {
        printf("%s test failed\n", cmdname.c_str());
      } else {
        printf("%s test passed\n", cmdname.c_str());
      }
    }
    total_error |= error;
  }
  return total_error;
}

bool test_asm_addi(bool verbose = false) {
  bool error = false;
  bool subtest_verbose = false;
  uint32_t cmd, exp;
  cmd = asm_addi(T0, T0, 0);
  exp = 0b00000000000000101000001010010011;
  error |= check_code("addi T0, T0, 0", cmd, exp, verbose);
  error |= test_i_type_decode(exp, OPCODE_ADDI, FUNC3_ADDI, T0, T0, 0,
                              subtest_verbose);

  cmd = asm_addi(T3, T4, 0b101010101010);
  exp = 0b10101010101011101000111000010011;
  error |= check_code("addi T3, T4, -1366", cmd, exp, verbose);
  error |= test_i_type_decode(exp, OPCODE_ADDI, FUNC3_ADDI, T3, T4, -1366,
                              subtest_verbose);

  if (verbose && error) {
    printf("Addi test failed\n");
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
  error |=
      test_b_type_decode(exp, OPCODE_B, FUNC3_BEQ, T0, T0, 0, subtest_verbose);

  cmd = asm_beq(T0, T0, 0b1010101010101);
  exp = 0b11010100010100101000101001100011;
  error |= check_code("beq T0, T0, -2732", cmd, exp, verbose);
  error |= test_b_type_decode(exp, OPCODE_B, FUNC3_BEQ, T0, T0, -2732,
                              subtest_verbose);

  cmd = asm_beq(T3, T4, 0b1010101010101);
  exp = 0b11010101110111100000101001100011;
  error |= check_code("beq T3, T4, -2732", cmd, exp, verbose);
  error |= test_b_type_decode(exp, OPCODE_B, FUNC3_BEQ, T3, T4, -2732,
                              subtest_verbose);

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
  error |= test_j_type_decode(exp, OPCODE_J, T0, 0, subtest_verbose);

  cmd = asm_jal(T4, 0b101010101010101010101);
  exp = 0b11010101010001010101111011101111;
  error |= check_code("jal T4, -699052", cmd, exp, verbose);
  error |= test_j_type_decode(exp, OPCODE_J, T4, -699052, subtest_verbose);

  cmd = asm_jal(ZERO, 0);
  exp = 0b00000000000000000000000001101111;
  error |= check_code("jal ZERO, 0", cmd, exp, verbose);
  error |= test_j_type_decode(exp, OPCODE_J, ZERO, 0, subtest_verbose);

  cmd = asm_jal(ZERO, -16);
  exp = 0b11111111000111111111000001101111;
  error |= check_code("jal ZERO, -16", cmd, exp, verbose);
  error |= test_j_type_decode(exp, OPCODE_J, ZERO, -16, subtest_verbose);

  if (verbose && error) {
    printf("JAL test failed\n");
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
  error |=
      test_i_type_decode(exp, OPCODE_LD, FUNC3_LD, T0, T0, 0, subtest_verbose);

  cmd = asm_ld(T3, T4, 0);
  exp = 0b00000000000011101011111000000011;
  error |= check_code("ld T3, T4, 0", cmd, exp, verbose);
  error |=
      test_i_type_decode(exp, OPCODE_LD, FUNC3_LD, T3, T4, 0, subtest_verbose);

  cmd = asm_ld(T3, T4, 2730);
  exp = 0b10101010101011101011111000000011;
  error |= check_code("ld T3, T4, -1366", cmd, exp, verbose);
  error |= test_i_type_decode(exp, OPCODE_LD, FUNC3_LD, T3, T4, -1366,
                              subtest_verbose);

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
  error |= test_i_type_decode(exp, OPCODE_JALR, FUNC3_JALR, T0, T0, 0,
                              subtest_verbose);

  cmd = asm_jalr(T3, T4, 0);
  exp = 0b00000000000011101000111001100111;
  error |= check_code("jalr T3, T4, 0", cmd, exp, verbose);
  error |= test_i_type_decode(exp, OPCODE_JALR, FUNC3_JALR, T3, T4, 0,
                              subtest_verbose);

  cmd = asm_jalr(T3, T4, 2730);
  exp = 0b10101010101011101000111001100111;
  error |= check_code("jalr T3, T4, -1366", cmd, exp, verbose);
  error |= test_i_type_decode(exp, OPCODE_JALR, FUNC3_JALR, T3, T4, -1366,
                              subtest_verbose);

  return error;
}

int main() {
  constexpr int SEED = 0;
  srand(SEED);
  bool verbose = true;
  bool error = false;
  error |= test_r_type(verbose);
  error |= test_asm_addi(verbose);
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