#include "RISCV_cpu.h"
#include "instruction_encdec.h"
#include "load_assembler.h"
#include <iostream>
#include <string>

using namespace std;

constexpr int TEST_NUM = 1000;

// Print binary bit by bit.
template <class T> void print_binary(T value) {
  int bitwidth = sizeof(T) * 8;
  for (int i = 0; i < bitwidth; i++) {
    printf("%d", (value >> (bitwidth - i - 1)) & 1);
    if (i % 8 == 7) {
      printf(" ");
    }
  }
}

// Compare the 
template <class T>
bool check_code(string text, T cmd, T exp, bool verbose = false) {
  bool error = cmd != exp;
  if (verbose) {
    cout << text;
    printf(": %04X (", cmd);
    print_binary(cmd);
    printf(")");
    if (error) {
      printf(" - Error (");
      print_binary(exp);
      printf(")\n");
    } else {
      printf(" - Pass\n");
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

// Only shows message when there's an error.
bool test_r_type_decode_quiet(uint32_t instruction, uint8_t opcode,
                              uint8_t funct3, uint8_t funct7, uint8_t rd,
                              uint8_t rs1, uint8_t rs2, bool verbose = false) {
  bool error = test_r_type_decode(instruction, opcode, funct3, funct7, rd, rs1,
                                  rs2, false);
  if (error) {
    // Show error message.
    error = test_r_type_decode(instruction, opcode, funct3, funct7, rd, rs1,
                               rs2, true);
  }
  return error;
}

bool test_i_type_decode(uint32_t instruction, uint8_t opcode, uint8_t funct3,
                        uint8_t rd, uint8_t rs1, int16_t imm12,
                        bool verbose = false) {
  bool error = false;
  i_type cmd;
  cmd.set_value(instruction);
  error |= check_equal("cmd", cmd.opcode, opcode, verbose);
  error |= check_equal("funct3", cmd.funct3, funct3, verbose);
  error |= check_equal("rd", cmd.rd, rd, verbose);
  error |= check_equal("rs1", cmd.rs1, rs1, verbose);
  error |= check_equal("imm12", cmd.imm12, imm12, verbose);
  return error;
}

// Only shows message when there's an error.
bool test_i_type_decode_quiet(uint32_t instruction, uint8_t opcode,
                              uint8_t funct3, uint8_t rd, uint8_t rs1,
                              int16_t imm12, bool verbose = false) {
  bool error =
      test_i_type_decode(instruction, opcode, funct3, rd, rs1, imm12, false);
  if (error) {
    // Show error message.
    error =
        test_i_type_decode(instruction, opcode, funct3, rd, rs1, imm12, true);
  }
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
  error |= check_equal("imm13", cmd.imm13, imm13 & (~0b01), verbose);
  return error;
}

// Only shows message when there's an error.
bool test_b_type_decode_quiet(uint32_t instruction, uint8_t opcode, uint8_t funct3,
                        uint8_t rs1, uint8_t rs2, int16_t imm13,
                        bool verbose = false) {
  bool error =
      test_b_type_decode(instruction, opcode, funct3, rs1, rs2, imm13, false);
  if (error) {
    // Show error message.
    error =
        test_b_type_decode(instruction, opcode, funct3, rs1, rs2, imm13, true);
  }
  return error;
}

bool test_j_type_decode(uint32_t instruction, uint8_t opcode, uint8_t rd,
                        int32_t imm21, bool verbose = false) {
  bool error = false;
  j_type cmd;
  cmd.set_value(instruction);
  error |= check_equal("cmd", cmd.opcode, opcode, verbose);
  error |= check_equal("rd", cmd.rd, rd, verbose);
  error |= check_equal("imm21", cmd.imm21, imm21 & (~1), verbose);
  return error;
}

// Only shows message when there's an error.
bool test_j_type_decode_quiet(uint32_t instruction, uint8_t opcode, uint8_t rd,
                        int32_t imm21, bool verbose = false) {
  bool error =
      test_j_type_decode(instruction, opcode, rd, imm21, false);
  if (error) {
    // Show error message.
    error =
        test_j_type_decode(instruction, opcode, rd, imm21, true);
  }
  return error;
}

bool test_s_type_decode(uint32_t instruction, uint8_t opcode, uint8_t funct3,
                        uint8_t rs1, uint8_t rs2, int16_t imm12,
                        bool verbose = false) {
  bool error = false;
  s_type cmd;
  cmd.set_value(instruction);
  error |= check_equal("cmd", cmd.opcode, opcode, verbose);
  error |= check_equal("funct3", cmd.funct3, funct3);
  error |= check_equal("rs1", cmd.rs1, rs1, verbose);
  error |= check_equal("rs2", cmd.rs2, rs2, verbose);
  error |= check_equal("imm12", cmd.imm12, imm12, verbose);
  return error;
}

// Only shows message when there's an error.
bool test_s_type_decode_quiet(uint32_t instruction, uint8_t opcode, uint8_t funct3,
                        uint8_t rs1, uint8_t rs2, int16_t imm12,
                        bool verbose = false) {
  bool error =
      test_s_type_decode(instruction, opcode, funct3, rs1, rs2, imm12, false);
  if (error) {
    // Show error message.
    error =
        test_s_type_decode(instruction, opcode, funct3, rs1, rs2, imm12, true);
  }
  return error;
}

void print_error_result(string &cmdname, int num_test, bool error, bool verbose) {
    if (verbose) {
      printf("Total %d %s random encode & decode test finished. ", num_test, cmdname.c_str());
      if (error) {
        printf("%s test failed\n", cmdname.c_str());
      } else {
        printf("%s test passed\n", cmdname.c_str());
      }
    }
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
      error |= test_r_type_decode_quiet(exp, opcode, funct3, funct7, rd, rs1,
                                        rs2, verbose);
    }
    print_error_result(cmdname, TEST_NUM, error, verbose);
    total_error |= error;
  }
  return total_error;
}

uint32_t gen_i_type(uint32_t base, uint8_t rd, uint8_t rs1, int16_t imm12) {
  return base | ((imm12 & 0xFFF) << 20) | ((rs1 & 0x1F) << 15) |
         ((rd & 0x1F) << 7);
}

bool test_i_type(bool verbose = false) {
  enum TEST_LIST { TEST_ADDI, TEST_LD, TEST_JALR };
  bool total_error = false;

  for (int testcase : {TEST_ADDI, TEST_LD, TEST_JALR}) {
    bool error = false;
    uint32_t base;
    string cmdname;
    uint8_t opcode, funct3;
    switch (testcase) {
    case TEST_ADDI:
      base = 0b00000000000000000000000000010011;
      cmdname = "ADDI";
      opcode = OPCODE_ADDI;
      funct3 = FUNC3_ADDI;
      break;
    case TEST_LD:
      base = 0b00000000000000000011000000000011;
      cmdname = "LD";
      opcode = OPCODE_LD;
      funct3 = FUNC3_LD;
      break;
    case TEST_JALR:
      base = 0b00000000000000000000000001100111;
      cmdname = "JALR";
      opcode = OPCODE_JALR;
      funct3 = FUNC3_JALR;
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
      int16_t imm12 = (rand() % (1 << 12)) - (1 << 11);
      switch (testcase) {
      case TEST_ADDI:
        cmd = asm_addi(rd, rs1, imm12);
        break;
      case TEST_LD:
        cmd = asm_ld(rd, rs1, imm12);
        break;
      case TEST_JALR:
        cmd = asm_jalr(rd, rs1, imm12);
        break;
      default:
        break;
      }
      uint32_t exp = gen_i_type(base, rd, rs1, imm12);
      string test_string = cmdname + " " + to_string(rd) + ", " +
                           to_string(rs1) + ", " + to_string(imm12);
      error |= check_code_quiet(test_string, cmd, exp, verbose);
      error |= test_i_type_decode_quiet(exp, opcode, funct3, rd, rs1, imm12,
                                        verbose);
    }
    print_error_result(cmdname, TEST_NUM, error, verbose);
    total_error |= error;
  }
  return total_error;
}

uint32_t gen_b_type(uint32_t base, uint8_t rs1, uint8_t rs2, int16_t imm13) {
  uint32_t instruction = base | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15);
  instruction |= ((imm13 >> 12) & 0b01) << 31;
  instruction |= ((imm13 >> 5) & 0b0111111) << 25;
  instruction |= ((imm13 >> 1) & 0b01111) << 8;
  instruction |= ((imm13 >> 11) & 0b01) << 7;
  return instruction;
}

bool test_b_type(bool verbose = false) {
  enum TEST_LIST { TEST_BEQ };
  bool total_error = false;

  for (int testcase : {TEST_BEQ}) {
    bool error = false;
    uint32_t base;
    string cmdname;
    uint8_t opcode, funct3;
    switch (testcase) {
    case TEST_BEQ:
      base = 0b00000000000000000000000001100011;
      cmdname = "BEQ";
      opcode = OPCODE_B;
      funct3 = FUNC3_BEQ;
      break;
    default:
      printf("Test case is node defined yet\n");
      return true;
      break;
    }

    for (int i = 0; i < TEST_NUM; i++) {
      uint32_t cmd;
      uint8_t rs1 = rand() % 32;
      uint8_t rs2 = rand() % 32;
      int16_t imm13 = (rand() % (1 << 13)) - (1 << 12);
      switch (testcase) {
      case TEST_BEQ:
        cmd = asm_beq(rs1, rs2, imm13);
        break;
      default:
        break;
      }
      uint32_t exp = gen_b_type(base, rs1, rs2, imm13);
      string test_string = cmdname + " " + to_string(rs1) + ", " +
                           to_string(rs2) + ", " + to_string(imm13);
      error |= check_code_quiet(test_string, cmd, exp, verbose);
      error |= test_b_type_decode_quiet(exp, opcode, funct3, rs1, rs2, imm13,
                                        verbose);
    }
    print_error_result(cmdname, TEST_NUM, error, verbose);
    total_error |= error;
  }
  return total_error;
}

uint32_t gen_j_type(uint32_t base, uint8_t rd, int32_t imm21) {
  uint32_t instruction = base | ((rd & 0x1F) << 7);
  instruction |= ((imm21 >> 20) & 0b01) << 31;
  instruction |= ((imm21 >> 1) & 0b01111111111) << 21;
  instruction |= ((imm21 >> 11) & 0b01) << 20;
  instruction |= ((imm21 >> 12) & 0b011111111) << 12;
  return instruction;
}

bool test_j_type(bool verbose = false) {
  enum TEST_LIST { TEST_JAL };
  bool total_error = false;

  for (int testcase : {TEST_JAL}) {
    bool error = false;
    uint32_t base;
    string cmdname;
    uint8_t opcode;
    switch (testcase) {
    case TEST_JAL:
      base = 0b00000000000000000000000001101111;
      cmdname = "JAL";
      opcode = OPCODE_J;
      break;
    default:
      printf("Test case is node defined yet\n");
      return true;
      break;
    }

    for (int i = 0; i < TEST_NUM; i++) {
      uint32_t cmd;
      uint8_t rd = rand() % 32;
      int32_t imm21 = (rand() % (1 << 21)) - (1 << 20);
      switch (testcase) {
      case TEST_JAL:
        cmd = asm_jal(rd, imm21);
        break;
      default:
        break;
      }
      uint32_t exp = gen_j_type(base, rd, imm21);
      string test_string = cmdname + " " + to_string(rd) + ", " +
                           ", " + to_string(imm21);
      error |= check_code_quiet(test_string, cmd, exp, verbose);
      error |= test_j_type_decode_quiet(exp, opcode, rd, imm21,
                                        verbose);
    }
    print_error_result(cmdname, TEST_NUM, error, verbose);
    total_error |= error;
  }
  return total_error;
}

uint32_t gen_s_type(uint32_t base, uint8_t rs1, uint8_t rs2, int16_t imm12) {
  uint32_t instruction = base | ((rs1 & 0x01F) << 15) | ((rs2 & 0x01F) << 20);
  instruction |= (((imm12 >> 5) & 0b01111111) << 25) | ((imm12 & 0b011111) << 7);
  return instruction;
}

bool test_s_type(bool verbose = false) {
  enum TEST_LIST { TEST_SW };
  bool total_error = false;

  for (int testcase : {TEST_SW}) {
    bool error = false;
    uint32_t base;
    string cmdname;
    uint8_t opcode;
    uint8_t funct3;
    switch (testcase) {
    case TEST_SW:
      base = 0b00000000000000000010000000100011;
      cmdname = "SW";
      opcode = OPCODE_S;
      funct3 = FUNC3_SW;
      break;
    default:
      printf("Test case is node defined yet\n");
      return true;
      break;
    }

    for (int i = 0; i < TEST_NUM; i++) {
      uint32_t cmd;
      uint8_t rs1 = rand() % 32;
      uint8_t rs2 = rand() % 32;
      int32_t imm12 = (rand() % (1 << 12)) - (1 << 11);
      switch (testcase) {
      case TEST_SW:
        cmd = asm_sw(rs1, rs2, imm12);
        break;
      default:
        break;
      }
      uint32_t exp = gen_s_type(base, rs1, rs2, imm12);
      string test_string = cmdname + " " + to_string(rs1) + ", " + to_string(rs2) + ", " +
                           ", " + to_string(imm12);
      error |= check_code_quiet(test_string, cmd, exp, verbose);
      error |= test_s_type_decode_quiet(exp, opcode, funct3, rs1, rs2, imm12,
                                        verbose);
    }
    print_error_result(cmdname, TEST_NUM, error, verbose);
    total_error |= error;
  }
  return total_error;
}


int main() {
  constexpr int SEED = 0;
  srand(SEED);
  bool verbose = true;
  bool error = false;
  error |= test_r_type(verbose);
  error |= test_i_type(verbose);
  error |= test_b_type(verbose);
  error |= test_j_type(verbose);
  error |= test_s_type(verbose);

  if (error) {
    printf("Test failed\n");
  } else {
    printf("Test passed\n");
  }
  return error;
}