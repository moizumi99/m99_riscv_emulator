
#include "RISCV_cpu.h"
#include "bit_tools.h"
#include "load_assembler.h"
#include "assembler.h"
#include <map>
#include <random>

namespace cpu_test {

constexpr int kMemSize = 0x0200000;
std::vector<uint8_t > memory;
uint8_t *mem;


std::mt19937 rnd;
constexpr int kSeed = 155719;

void init_random() {
  rnd.seed(kSeed);
}

// The number of test cases for each command.
constexpr int kUnitTestMax = 100;

//  memory initialization
void mem_init() {
  memory.resize(kMemSize);
  mem = memory.data();
}

void randomize_registers(RiscvCpu &cpu) {
  std::mt19937_64 gen(std::random_device{}());

  constexpr int kRegNumber = 32;
  for (int i = 1; i < kRegNumber; i++) {
    uint32_t reg = gen() & 0xFFFFFFFF;
    cpu.set_register(i, reg);
  }
}

// Commonly used helper function for error message.
void print_error_message(const std::string &text, bool error, int32_t expected, int32_t actual) {
  if (error) {
    printf("%s test failed.", text.c_str());
  } else {
    printf("%s test passed.", text.c_str());

  }
  printf(" Expected %08x, Actual %08x\n", expected, actual);
  printf(" Expected %d, Actual %d\n", expected, actual);
}

// A helper function to split 32 bit value to 20 bit and singed 12 bit immediates.
std::pair<uint32_t, uint32_t> split_immediate(uint32_t value) {
  uint32_t value20 = value >> 12;
  uint32_t value12 = value & 0xFFF;
  if (value12 >> 11) {
    value20++;
  }
  // Double check that the sum of these two equals the input.
  if ((value20 << 12) + sext(value12, 12) != value) {
    printf("Test bench error: value1 = %8X, value20 = %8X, value12 = %8X\n", value, value20, value12);
  }
  return std::make_pair(value20, value12);
}

// I TYPE test cases starts here.
enum ITYPE_TEST {
  TEST_ADDI, TEST_ANDI, TEST_ORI, TEST_XORI, TEST_SLLI, TEST_SRLI, TEST_SRAI, TEST_SLTI, TEST_SLTIU, TEST_EBREAK
};

bool test_i_type(ITYPE_TEST test_type, int32_t rd, int32_t rs1, int32_t value, int32_t imm12, bool verbose) {
  bool error = false;
  int32_t expected;
  std::string test_case = "";

  // CPU is instantiated here because some tests need access to cpu register.
  RiscvCpu cpu;
  randomize_registers(cpu);
  uint8_t *pointer = mem;
  uint32_t val20, val12;
  std::tie(val20, val12) = split_immediate(value);
  pointer = add_cmd(pointer, asm_lui(rs1, val20));
  pointer = add_cmd(pointer, asm_addi(rs1, rs1, val12));
  if (rs1 == 0) {
    value = 0;
  }
  switch (test_type) {
    case TEST_ADDI:
      pointer = add_cmd(pointer, asm_addi(rd, rs1, imm12));
      expected = value + sext(imm12 & 0x0FFF, 12);
      test_case = "ADDI";
      break;
    case TEST_ANDI:
      pointer = add_cmd(pointer, asm_andi(rd, rs1, imm12));
      expected = value & sext(imm12 & 0x0FFF, 12);
      test_case = "ANDI";
      break;
    case TEST_ORI:
      pointer = add_cmd(pointer, asm_ori(rd, rs1, imm12));
      expected = value | sext(imm12 & 0x0FFF, 12);
      test_case = "ORI";
      break;
    case TEST_XORI:
      pointer = add_cmd(pointer, asm_xori(rd, rs1, imm12));
      expected = value ^ sext(imm12 & 0x0FFF, 12);
      test_case = "XORI";
      break;
    case TEST_SLLI:
      imm12 = imm12 & 0b0011111;
      pointer = add_cmd(pointer, asm_slli(rd, rs1, imm12));
      expected = value << imm12;
      test_case = "SLLI";
      break;
    case TEST_SRLI:
      imm12 = imm12 & 0b0011111;
      pointer = add_cmd(pointer, asm_srli(rd, rs1, imm12));
      expected = static_cast<uint32_t>(value) >> imm12;
      test_case = "SRLI";
      break;
    case TEST_SRAI:
      imm12 = imm12 & 0b0011111;
      pointer = add_cmd(pointer, asm_srai(rd, rs1, imm12));
      expected = value >> imm12;
      test_case = "SRAI";
      break;
    case TEST_SLTI:
      pointer = add_cmd(pointer, asm_slti(rd, rs1, imm12));
      expected = value < imm12 ? 1 : 0;
      test_case = "SLTI";
      break;
    case TEST_SLTIU:
      pointer = add_cmd(pointer, asm_sltiu(rd, rs1, imm12));
      expected = static_cast<uint32_t>(value) < static_cast<uint32_t >(imm12) ? 1 : 0;
      test_case = "SLTIU";
      break;
    case TEST_EBREAK:
      pointer = add_cmd(pointer, asm_ebreak());
      expected = cpu.read_register(rd);
      if (rs1 == rd) {
        expected = value;
      }
      test_case = "EBREAK";
      break;
    default:
      printf("I TYPE Test case undefined.\n");
      return true;
  }
  pointer = add_cmd(pointer, asm_addi(A0, rd, 0));
  pointer = add_cmd(pointer, asm_xor(RA, RA, RA));
  pointer = add_cmd(pointer, asm_jalr(ZERO, RA, 0));

  if (rd == 0) {
    expected = 0;
  }
  cpu.set_memory(memory);
  error = cpu.run_cpu(0, verbose) != 0;
  int return_value = cpu.read_register(A0);
  error |= return_value != expected;
  if (error & verbose) {
    printf("RD: %d, RS1: %d, Value: %d(%08x), imm12: %d(%03x)\n", rd, rs1, value, value, imm12, imm12);
  }
  if (verbose) {
    print_error_message(test_case, error, expected, return_value);
  }
  return error;
}

void print_i_type_instruction_message(ITYPE_TEST test_case, bool error) {
  std::map<ITYPE_TEST, const std::string> test_name = {{TEST_ADDI,  "ADDI"},
                                                       {TEST_ANDI,  "ANDI"},
                                                       {TEST_ORI,   "ORI"},
                                                       {TEST_XORI,  "XORI"},
                                                       {TEST_SLLI,  "SLLI"},
                                                       {TEST_SRLI,  "SRLI"},
                                                       {TEST_SRAI,  "SRAI"},
                                                       {TEST_SLTI,  "SLTI"},
                                                       {TEST_SLTIU, "SLTIU"},
                                                       {TEST_EBREAK, "EBREAK"}};
  printf("%s test %s.\n", test_name[test_case].c_str(), error ? "failed" : "passed");
}

bool test_i_type_loop(bool verbose) {
  bool total_error = false;
  ITYPE_TEST test_set[] = {TEST_ADDI, TEST_ANDI, TEST_ORI, TEST_XORI, TEST_SLLI, TEST_SRLI, TEST_SRAI, TEST_SLTI,
                           TEST_SLTIU, TEST_EBREAK};
  for (ITYPE_TEST test_case: test_set) {
    bool error = false;
    for (int i = 0; i < kUnitTestMax && !error; i++) {
      uint32_t rd = rnd() & 0x1F;
      uint32_t rs1 = rnd() & 0x1F;
      int32_t value = rnd();
      int32_t imm12 = sext(rnd() & 0x0FFF, 12);
      bool test_error = test_i_type(test_case, rd, rs1, value, imm12, false);
      if (test_error) {
        test_error |= test_i_type(test_case, rd, rs1, value, imm12, true);
      }
      error |= test_error;
    }
    if (verbose) {
      print_i_type_instruction_message(test_case, error);
    }
    total_error |= error;
  }
  return total_error;
}
// I-Type test cases end here.

// R-Type test cases start here.
enum R_TYPE_TEST {
  TEST_ADD, TEST_SUB, TEST_AND, TEST_OR, TEST_XOR, TEST_SLL, TEST_SRL, TEST_SRA, TEST_SLT, TEST_SLTU,
};

bool
test_r_type(R_TYPE_TEST test_type, int32_t rd, int32_t rs1, int32_t rs2, int32_t value1, int32_t value2,
            bool verbose) {
  bool error = false;
  int32_t expected;
  std::string test_case = "";

  uint8_t *pointer = mem;
  uint32_t val20, val12;
  std::tie(val20, val12) = split_immediate(value1);
  pointer = add_cmd(pointer, asm_lui(rs1, val20));
  pointer = add_cmd(pointer, asm_addi(rs1, rs1, val12));
  std::tie(val20, val12) = split_immediate(value2);
  pointer = add_cmd(pointer, asm_lui(rs2, val20));
  pointer = add_cmd(pointer, asm_addi(rs2, rs2, val12));

  if (rs1 == 0) {
    value1 = 0;
  }
  if (rs2 == 0) {
    value2 = 0;
  }
  if (rs1 == rs2) {
    value1 = value2;
  }
  switch (test_type) {
    case TEST_ADD:
      pointer = add_cmd(pointer, asm_add(rd, rs1, rs2));
      expected = value1 + value2;
      test_case = "ADD";
      break;
    case TEST_SUB:
      pointer = add_cmd(pointer, asm_sub(rd, rs1, rs2));
      expected = value1 - value2;
      test_case = "SUB";
      break;
    case TEST_AND:
      pointer = add_cmd(pointer, asm_and(rd, rs1, rs2));
      expected = value1 & value2;
      test_case = "AND";
      break;
    case TEST_OR:
      pointer = add_cmd(pointer, asm_or(rd, rs1, rs2));
      expected = value1 | value2;
      test_case = "OR";
      break;
    case TEST_XOR:
      pointer = add_cmd(pointer, asm_xor(rd, rs1, rs2));
      expected = value1 ^ value2;
      test_case = "XOR";
      break;
    case TEST_SLL:
      pointer = add_cmd(pointer, asm_sll(rd, rs1, rs2));
      expected = value1 << (value2 & 0x1F);
      test_case = "SLL";
      break;
    case TEST_SRL:
      pointer = add_cmd(pointer, asm_srl(rd, rs1, rs2));
      expected = static_cast<uint32_t>(value1) >> (value2 & 0x1F);
      test_case = "SRL";
      break;
    case TEST_SRA:
      pointer = add_cmd(pointer, asm_sra(rd, rs1, rs2));
      expected = value1 >> (value2 & 0x1F);
      test_case = "SRA";
      break;
    case TEST_SLT:
      pointer = add_cmd(pointer, asm_slt(rd, rs1, rs2));
      expected = (value1 < value2) ? 1 : 0;
      test_case = "SLT";
      break;
    case TEST_SLTU:
      pointer = add_cmd(pointer, asm_sltu(rd, rs1, rs2));
      expected = (static_cast<uint32_t>(value1) < static_cast<uint32_t>(value2)) ? 1 : 0;
      test_case = "SLTU";
      break;
    default:
      if (verbose) {
        printf("Undefined test case.\n");
      }
      return true;
  }
  pointer = add_cmd(pointer, asm_addi(A0, rd, 0));
  pointer = add_cmd(pointer, asm_xor(RA, RA, RA));
  add_cmd(pointer, asm_jalr(ZERO, RA, 0));

  if (rd == 0) {
    expected = 0;
  }
  RiscvCpu cpu;
  randomize_registers(cpu);
  cpu.set_memory(memory);
  error = cpu.run_cpu(0, verbose) != 0;
  int return_value = cpu.read_register(A0);
  error |= return_value != expected;
  if (error & verbose) {
    printf("RD: %d, RS1: %d, RS2: %d, Value1: %d(%08x), value2: %d(%03x)\n", rd, rs1, rs2, value1, value1,
           value2, value2);
  }
  if (verbose) {
    print_error_message(test_case, error, expected, return_value);
  }
  return error;
}

void print_r_type_instruction_message(R_TYPE_TEST test_case, bool error) {
  std::map<R_TYPE_TEST, const std::string> test_name = {{TEST_ADD,  "ADD"},
                                                        {TEST_SUB,  "SUB"},
                                                        {TEST_AND,  "AND"},
                                                        {TEST_OR,   "OR"},
                                                        {TEST_XOR,  "XOR"},
                                                        {TEST_SLL,  "SLL"},
                                                        {TEST_SRL,  "SRL"},
                                                        {TEST_SRA,  "SRA"},
                                                        {TEST_SLT,  "SLT"},
                                                        {TEST_SLTU, "SLTU"}};
  printf("%s test %s.\n", test_name[test_case].c_str(), error ? "failed" : "passed");
}

bool test_r_type_loop(bool verbose = true) {
  bool total_error = false;
  R_TYPE_TEST test_sets[] = {TEST_ADD, TEST_SUB, TEST_AND, TEST_OR, TEST_XOR, TEST_SLL, TEST_SRL, TEST_SRA,
                             TEST_SLT, TEST_SLTU};
  for (R_TYPE_TEST test_case: test_sets) {
    bool error = false;
    for (int i = 0; i < kUnitTestMax && !error; i++) {
      int32_t rd = rnd() & 0x1F;
      int32_t rs1 = rnd() & 0x1F;
      int32_t rs2 = rnd() & 0x1F;
      int32_t value1 = static_cast<int32_t>(rnd());
      int32_t value2 = static_cast<int32_t>(rnd());
      bool test_error = test_r_type(test_case, rd, rs1, rs2, value1, value2, false);
      if (test_error && verbose) {
        test_error = test_r_type(test_case, rd, rs1, rs2, value1, value2, true);
      }
      error |= test_error;
    }
    if (verbose) {
      print_r_type_instruction_message(test_case, error);
    }
    total_error |= error;
  }
  return total_error;
}
// R-Type test cases end here.

// AUIPC has it's own test set, starting here.
bool test_auipc(int32_t rd, int32_t val, int32_t offset, bool verbose) {
  uint8_t *pointer = mem;
  add_cmd(pointer, asm_jal(ZERO, offset));
  pointer = mem + offset;
  pointer = add_cmd(pointer, asm_auipc(rd, val));
  pointer = add_cmd(pointer, asm_addi(A0, rd, 0));
  pointer = add_cmd(pointer, asm_xor(RA, RA, RA));
  pointer = add_cmd(pointer, asm_jalr(ZERO, RA, 0));

  int32_t expected = offset + (val << 12);
  if (rd == 0) {
    expected = 0;
  }
  RiscvCpu cpu;
  randomize_registers(cpu);
  cpu.set_memory(memory);
  bool error = cpu.run_cpu(0, verbose) != 0;
  int return_value = cpu.read_register(A0);
  error |= return_value != expected;
  if (verbose) {
    print_error_message("AUIPC", error, expected, return_value);
  }
  return error;
}

bool test_auipc_loop(bool verbose) {
  bool error = false;
  for (int i = 0; i < kUnitTestMax && !error; i++) {
    int32_t value = static_cast<int32_t>(rnd()) & 0x0FFFFF;
    int32_t offset = static_cast<uint32_t>(rnd()) & 0x0FFF0;
    int32_t rd = rnd() % 32;
    bool test_error = test_auipc(rd, value, offset, false);
    if (test_error && verbose) {
      test_error |= test_auipc(rd, value, offset, true);
    }
    error |= test_error;
  }
  if (verbose) {
    printf("AUIPC test %s.\n", error ? "failed" : "passed");
  }
  return error;
}
// AUIPC test cases end here.

// LUI has its own test cases, starting here.
bool test_lui(int32_t val, bool verbose) {
  // LUI test code
  uint8_t *pointer = mem;
  pointer = add_cmd(pointer, asm_add(A0, ZERO, 0));
  pointer = add_cmd(pointer, asm_lui(A0, val >> 12));
  pointer = add_cmd(pointer, asm_xor(RA, RA, RA));
  pointer = add_cmd(pointer, asm_jalr(ZERO, RA, 0));

  int32_t expected = val & 0xFFFFF000;
  RiscvCpu cpu;
  randomize_registers(cpu);
  cpu.set_memory(memory);
  bool error = cpu.run_cpu(0, verbose) != 0;
  int return_value = cpu.read_register(A0);
  error |= return_value != expected;
  if (verbose) {
    print_error_message("LUI", error, expected, return_value);
  }
  return error;
}

bool test_lui_loop(bool verbose) {
  bool error = false;
  for (int i = 0; i < kUnitTestMax && !error; i++) {
    int32_t value = static_cast<int32_t>(rnd());
    bool test_error = test_lui(value, false);
    if (test_error && verbose) {
      test_error = test_lui(value, true);
    }
    error |= test_error;
  }
  if (verbose) {
    printf("LUI test %s.\n", error ? "failed" : "passed");
  }
  return error;
}
// LUI test cases end here.

// LOAD test cases start here.
enum LOAD_TEST {
  TEST_LB, TEST_LBU, TEST_LH, TEST_LHU, TEST_LW
};

bool test_load(LOAD_TEST test_type, uint32_t rd, uint32_t rs1, uint32_t offset0, uint32_t offset1, uint32_t val,
               bool verbose) {
  bool error = false;
  std::string test_case = "";

  if (rs1 == ZERO) {
    offset0 = 0;
  }
  uint32_t address = offset0 + sext(offset1, 12);
  mem[address] = val & 0xff;
  mem[address + 1] = (val >> 8) & 0xff;
  mem[address + 2] = (val >> 16) & 0xff;
  mem[address + 3] = (val >> 24) & 0xff;
  // LW test code
  uint8_t *pointer = mem;
  int32_t val20, val12;
  std::tie(val20, val12) = split_immediate(offset0);
  int32_t expected;
  pointer = add_cmd(pointer, asm_lui(rs1, val20));
  pointer = add_cmd(pointer, asm_addi(rs1, rs1, val12));
  switch (test_type) {
    case TEST_LW:
      pointer = add_cmd(pointer, asm_lw(rd, rs1, offset1));
      expected = val;
      break;
    case TEST_LB:
      pointer = add_cmd(pointer, asm_lb(rd, rs1, offset1));
      expected = sext(val & 0xFF, 8);
      break;
    case TEST_LBU:
      pointer = add_cmd(pointer, asm_lbu(rd, rs1, offset1));
      expected = val & 0xFF;
      break;
    case TEST_LH:
      pointer = add_cmd(pointer, asm_lh(rd, rs1, offset1));
      expected = sext(val & 0xFFFF, 8);
      break;
    case TEST_LHU:
      pointer = add_cmd(pointer, asm_lhu(rd, rs1, offset1));
      expected = val & 0xFFFF;
      break;
    default:
      if (verbose) {
        printf("Undefined test case %d\n", test_type);
        return true;
      }
  }
  pointer = add_cmd(pointer, asm_addi(A0, rd, 0));
  pointer = add_cmd(pointer, asm_xor(RA, RA, RA));
  pointer = add_cmd(pointer, asm_jalr(ZERO, RA, 0));
  expected = (rd == ZERO) ? 0 : expected;

  RiscvCpu cpu;
  randomize_registers(cpu);
  cpu.set_memory(memory);
  error = cpu.run_cpu(0, verbose) != 0;
  int return_value = cpu.read_register(A0);
  error |= return_value != expected;
  if (verbose) {
    print_error_message(test_case, error, expected, return_value);
    if (error) {
      printf("rd: %2d, rs1: %2d, offset0: %08X, offset1: %08X, val: %08X\n", rd, rs1, offset0, offset1, val);
    }
  }
  return error;
}

void print_load_instruction_message(LOAD_TEST test_case, bool error, bool verbose = true) {
  if (!verbose) {
    return;
  }
  std::map<LOAD_TEST, const std::string> test_name = {{TEST_LB,  "LB"},
                                                      {TEST_LBU, "LBU"},
                                                      {TEST_LH,  "LH"},
                                                      {TEST_LHU, "LHU"},
                                                      {TEST_LW,  "LW"},};
  printf("%s test %s.\n", test_name[test_case].c_str(), error ? "failed" : "passed");
}

bool test_load_loop(bool verbose) {
  bool error = false;
  LOAD_TEST test_sets[] = {TEST_LB, TEST_LBU, TEST_LH, TEST_LHU, TEST_LW};
  for (auto test_case: test_sets) {
    for (int i = 0; i < kUnitTestMax && !error; i++) {
      uint32_t rs1 = rnd() % 32;
      uint32_t rd = rnd() % 32;
      uint32_t offset0 = 0;
      int32_t offset1 = 0, offset = 0;
      while (offset < 32 || offset >= kMemSize - 4) {
        uint32_t offset0_effective;
        offset0 = rnd() % kMemSize;
        offset1 = rnd() & 0x0FFF;
        offset0_effective = (rs1 == ZERO) ? 0 : offset0;
        offset = offset0_effective + sext(offset1, 12);
      }
      uint32_t val = rnd() & 0x0FFFFFFFF;
      bool test_error = test_load(test_case, rd, rs1, offset0, offset1, val, false);
      if (test_error && verbose) {
        test_error = test_load(test_case, rd, rs1, offset0, offset1, val, true);
      }
      error |= test_error;
    }
    print_load_instruction_message(test_case, error, verbose);
  }
  return error;
}
// LOAD test cases end here.

// Store test cases start here.
enum STORE_TEST {
  TEST_SW, TEST_SH, TEST_SB
};

bool
test_store(STORE_TEST test_type, uint32_t rs1, uint32_t rs2, uint32_t offset0, uint32_t offset1, uint32_t value,
           bool verbose) {
  bool error = false;
  std::string test_case = "";

  // STORE test code
  uint8_t *pointer = mem;
  uint32_t val20, val12;
  std::tie(val20, val12) = split_immediate(value);
  pointer = add_cmd(pointer, asm_lui(rs2, val20));
  pointer = add_cmd(pointer, asm_addi(rs2, rs2, val12));
  uint32_t offset20, offset12;
  std::tie(offset20, offset12) = split_immediate(offset0);
  pointer = add_cmd(pointer, asm_lui(rs1, offset20));
  pointer = add_cmd(pointer, asm_addi(rs1, rs1, offset12));
  pointer = add_cmd(pointer, asm_sw(rs1, rs2, offset1));
  int32_t expected;
  switch (test_type) {
    case TEST_SW:
      test_case = "SW";
      pointer = add_cmd(pointer, asm_sw(rs1, rs2, offset1));
      expected = value;
      break;
    case TEST_SH:
      test_case = "SH";
      pointer = add_cmd(pointer, asm_sh(rs1, rs2, offset1));
      expected = value & 0x0000FFFF;
      break;
    case TEST_SB:
      test_case = "SB";
      pointer = add_cmd(pointer, asm_sb(rs1, rs2, offset1));
      expected = value & 0x000000FF;
      break;
    default:
      if (verbose) {
        printf("Undefined test case %d\n", test_type);
      }
      return true;
  }
  pointer = add_cmd(pointer, asm_addi(A0, ZERO, 0));
  pointer = add_cmd(pointer, asm_xor(RA, RA, RA));
  pointer = add_cmd(pointer, asm_jalr(ZERO, RA, 0));
  expected = (rs2 == ZERO) ? 0 : expected;

  RiscvCpu cpu;
  randomize_registers(cpu);
  cpu.set_memory(memory);
  error = cpu.run_cpu(0, verbose) != 0;
  offset0 = (rs1 == ZERO) ? 0 : offset0;
  uint32_t address = offset0 + sext(offset1, 12);
  int32_t result = mem[address];
  result |= (test_type == TEST_SH || test_type == TEST_SW) ? (mem[address + 1] << 8) : 0;
  result |= (test_type == TEST_SW) ? (mem[address + 2] << 16) | (mem[address + 3] << 24) : 0;
  error |= result != expected;
  if (verbose) {
    print_error_message(test_case, error, expected, result);
    if (error) {
      printf("rs1: %2d, rs2: %2d, offset0: %08X, offset1: %08X, val: %08X\n", rs1, rs2, offset0, offset1,
             value);
    }
  }
  return error;
}

void print_store_instruction_message(STORE_TEST test_case, bool error, bool verbose = true) {
  if (!verbose) {
    return;
  }
  std::map<STORE_TEST, const std::string> test_name = {{TEST_SW, "SW"},
                                                       {TEST_SH, "SH"},
                                                       {TEST_SB, "SB"}};
  printf("%s test %s.\n", test_name[test_case].c_str(), error ? "failed" : "passed");
}

bool test_store_loop(bool verbose) {
  bool error = false;
  STORE_TEST test_sets[] = {TEST_SW, TEST_SH, TEST_SB};
  for (auto test_case: test_sets) {
    for (int i = 0; i < kUnitTestMax && !error; i++) {
      int32_t rs1 = rnd() % 32;
      int32_t rs2 = rnd() % 32;
      uint32_t offset0 = 0, offset1 = 0, offset = 0;
      while (offset < 16 || offset >= kMemSize - 4) {
        int32_t offset0_effective;
        offset0 = rnd() % kMemSize;
        offset1 = rnd() & 0x0FFF;
        offset0_effective = (rs1 == ZERO) ? 0 : offset0;
        offset = offset0_effective + sext(offset1, 12);
      }
      int32_t value = rnd() & 0xFFFFFFFF;
      if (rs1 == rs2) {
        value = offset0;
      }
      bool test_error = test_store(test_case, rs1, rs2, offset0, offset1, value, false);
      if (test_error && verbose) {
        test_error = test_store(test_case, rs1, rs2, offset0, offset1, value, true);
      }
      error |= test_error;
    }
    print_store_instruction_message(test_case, error, verbose);
  }
  return error;
}
// Store test cases end here.

// B-Type tests start here
enum B_TYPE_TEST {
  TEST_BEQ, TEST_BGE, TEST_BGEU, TEST_BLT, TEST_BLTU, TEST_BNE
};

std::map<B_TYPE_TEST, const std::string> test_name = {{TEST_BEQ,  "BEQ"},
                                                      {TEST_BGE,  "BGE"},
                                                      {TEST_BGEU, "BGEU"},
                                                      {TEST_BLT,  "BLT"},
                                                      {TEST_BLTU, "BLTU"},
                                                      {TEST_BNE,  "BNE"}};

void print_b_type_instruction_message(B_TYPE_TEST test_case, bool error) {
  printf("%s test %s.\n", test_name[test_case].c_str(), error ? "failed" : "passed");
}

bool
test_b_type(B_TYPE_TEST test_type, uint32_t rs1, uint32_t rs2, uint32_t value1, uint32_t value2, int32_t offset,
            bool verbose = true) {
  bool error = false;
  std::string test_case = test_name[test_type];

  value1 = (rs1 == ZERO) ? 0 : value1;
  value2 = (rs2 == ZERO) ? 0 : value2;
  value1 = (rs1 == rs2) ? value2 : value1;

  uint32_t start_point = kMemSize / 2;
  uint8_t *pointer = mem + start_point;
  uint32_t value20, value12;
  uint32_t expected;
  std::tie(value20, value12) = split_immediate(value1);
  pointer = add_cmd(pointer, asm_lui(rs1, value20));
  pointer = add_cmd(pointer, asm_addi(rs1, rs1, value12));
  std::tie(value20, value12) = split_immediate(value2);
  pointer = add_cmd(pointer, asm_lui(rs2, value20));
  pointer = add_cmd(pointer, asm_addi(rs2, rs2, value12));
  uint8_t *next_pos = pointer + offset;
  if (test_type == TEST_BEQ) {
    pointer = add_cmd(pointer, asm_beq(rs1, rs2, offset));
    expected = (value1 == value2) ? 1 : 0;
  } else if (test_type == TEST_BGE) {
    pointer = add_cmd(pointer, asm_bge(rs1, rs2, offset));
    expected = (static_cast<int32_t>(value1) >= static_cast<int32_t>(value2)) ? 1 : 0;
  } else if (test_type == TEST_BGEU) {
    pointer = add_cmd(pointer, asm_bgeu(rs1, rs2, offset));
    expected = value1 >= value2 ? 1 : 0;
  } else if (test_type == TEST_BLT) {
    pointer = add_cmd(pointer, asm_blt(rs1, rs2, offset));
    expected = static_cast<int32_t>(value1) < static_cast<int32_t>(value2) ? 1 : 0;
  } else if (test_type == TEST_BLTU) {
    pointer = add_cmd(pointer, asm_bltu(rs1, rs2, offset));
    expected = value1 < value2 ? 1 : 0;
  } else if (test_type == TEST_BNE) {
    pointer = add_cmd(pointer, asm_bne(rs1, rs2, offset));
    expected = value1 != value2 ? 1 : 0;
  }
  pointer = add_cmd(pointer, asm_addi(A0, ZERO, 0));
  pointer = add_cmd(pointer, asm_xor(RA, RA, RA));
  add_cmd(pointer, asm_jalr(ZERO, RA, 0));
  pointer = next_pos;
  pointer = add_cmd(pointer, asm_addi(A0, ZERO, 1));
  pointer = add_cmd(pointer, asm_xor(RA, RA, RA));
  add_cmd(pointer, asm_jalr(ZERO, RA, 0));


  RiscvCpu cpu;
  randomize_registers(cpu);
  cpu.set_memory(memory);
  error = cpu.run_cpu(start_point, verbose) != 0;
  int return_value = cpu.read_register(A0);
  error |= return_value != expected;
  if (error & verbose) {
    printf("RS1: %d, RS2: %d, value1: %d(%08x), value2: %d(%08x), offset: %d(%03x)\n",
           rs1, rs2, value1, value1, value2, value2, offset, offset);
  }

  if (verbose) {
    print_error_message(test_case, error, expected, return_value);
  }

  return error;
}

bool test_b_type_loop(bool verbose = true) {
  bool total_error = false;
  B_TYPE_TEST test_sets[] = {TEST_BEQ, TEST_BGE, TEST_BGEU, TEST_BLT, TEST_BLTU, TEST_BNE};

  for (B_TYPE_TEST test_case: test_sets) {
    bool error = false;
    for (int i = 0; i < kUnitTestMax && !error; i++) {
      uint32_t rs1 = rnd() % 32;
      uint32_t rs2 = rnd() % 32;
      uint32_t value1;
      uint32_t value2;
      uint32_t equal = rnd() % 2;
      switch (test_case) {
        case TEST_BEQ:
        case TEST_BNE:
          value1 = rnd();
          // You can ignore the corner case new random value equals to value1.
          value2 = equal ? value1 : rnd();
          break;
        case TEST_BGE:
        case TEST_BGEU:
        case TEST_BLT:
        case TEST_BLTU:
          value1 = static_cast<uint32_t>(rnd());
          value2 = static_cast<uint32_t>(rnd());
          break;
//                    default:
//                        if (verbose) {
//                            printf("Undefined test case %d\n", test_case);
//                        }
//                        return true;
      }
      int32_t offset = 0;
      while (-64 < offset && offset < 64) {
        constexpr int kRange = 1 << 12;
        offset = 2 * ((rnd() % kRange) - kRange / 2);
      }
      bool test_error = test_b_type(test_case, rs1, rs2, value1, value2, offset, false);
      if (test_error) {
        test_error = test_b_type(test_case, rs1, rs2, value1, value2, offset, true);
      }
      error |= test_error;
    }
    total_error |= error;
    if (verbose) {
      print_b_type_instruction_message(test_case, error);
    }
  }
  return total_error;
}
// B-Type tests end here.

void print_jalr_type_instruction_message(bool error) {
  printf("%s test %s.\n", "JALR", error ? "failed" : "passed");
}

bool test_jalr_type(uint32_t rd, uint32_t rs1, uint32_t offset, uint32_t value, bool verbose) {
  std::string test_case = "JALR";

  if (rs1 == 0) {
    value = 0;
  }
  uint32_t start_point = kMemSize / 4;
  uint8_t *pointer = mem + start_point;

  uint32_t value20, value12;
  std::tie(value20, value12) = split_immediate(value);
  pointer = add_cmd(pointer, asm_lui(rs1, value20));
  pointer = add_cmd(pointer, asm_addi(rs1, rs1, value12));
  pointer = add_cmd(pointer, asm_jalr(rd, rs1, offset));
  pointer = add_cmd(pointer, asm_addi(A0, ZERO, 1));
  pointer = add_cmd(pointer, asm_xor(RA, RA, RA));
  add_cmd(pointer, asm_jalr(ZERO, RA, 0));
  pointer = mem + ((value + sext(offset, 12)) & ~1);
  pointer = add_cmd(pointer, asm_addi(A0, ZERO, 2));
  pointer = add_cmd(pointer, asm_xor(RA, RA, RA));
  add_cmd(pointer, asm_jalr(ZERO, RA, 0));
  uint32_t expected = 2;
  RiscvCpu cpu;
  randomize_registers(cpu);
  cpu.set_memory(memory);
  bool error = cpu.run_cpu(start_point, verbose) != 0;
  int return_value = cpu.read_register(A0);
  error |= return_value != expected;
  if (rd != 0 && rd != RA && rd!= A0) {
    uint32_t expect = start_point + 12;
    uint32_t actual = cpu.read_register(rd);
    error |= actual != expect;
    if (actual != expect) {
      printf("reg[rd] = %d(%08x), expected = %d(%08x)\n", actual, actual, expect, expect);
    }
  }
  if (error & verbose) {
    printf("RS1: %d, RD: %d, value: %d(%08x), offset: %d(%03x)\n",
           rs1, rd, value, value, offset, offset);
  }

  if (verbose) {
    print_error_message(test_case, error, expected, return_value);
  }

  return error;
}

bool test_jalr_type_loop(bool verbose = true) {
  bool error = false;
  for (int i = 0; i < kUnitTestMax && !error; i++) {
    uint32_t rs1 = rnd() % 32;
    uint32_t rd = rnd() % 32;
    uint32_t offset = rnd() % 0x1000;
    // if rs1 == 0, the offset is from address=0. Only positive values are valid.
    if (rs1 == 0) {
      offset &= 0x7FF;
    }
    uint32_t value = kMemSize / 2 + (rnd() % (kMemSize / 4));
    bool test_error = test_jalr_type(rd, rs1, offset, value, false);
    if (test_error) {
      test_error = test_jalr_type(rd, rs1, offset, value, true);
    }
    error |= test_error;
  }
  if (verbose) {
    print_jalr_type_instruction_message(error);
  }
  return error;
}

// Summation test starts here.
bool test_sum(bool verbose) {
  load_assembler_sum(mem);
  constexpr int kExpectedValue = 55;
  RiscvCpu cpu;
  randomize_registers(cpu);
  cpu.set_memory(memory);
  bool error = cpu.run_cpu(0, verbose) != 0;
  int return_value = cpu.read_register(A0);
  error |= return_value != kExpectedValue;
  if (verbose) {
    print_error_message("Summation", error, kExpectedValue, return_value);
  }
  return error;
}

bool test_sum_quiet(bool verbose) {
  bool error = test_sum(false);
  if (error & verbose) {
    error = test_sum(true);
  }
  if (verbose) {
    printf("SUM test %s.\n", error ? "failed" : "passed");
  }
  return error;
}
// Summation test ends here.

// Sort test starts here.
bool test_sort(bool verbose) {
  load_assembler_sort(mem);

  constexpr int kArraySize = 100;
  constexpr int kArrayAddress = 512;
  for (int i = 0; i < kArraySize; i++) {
    int value = rnd() % 1000;
    store_wd(mem + kArrayAddress + i * 4, value);
  }

  if (verbose) {
    printf("Before:\n");
    for (int i = 0; i < kArraySize; i++) {
      printf("%d\t", load_wd(mem + kArrayAddress + i * 4));
    }
    printf("\n");
  }

  RiscvCpu cpu;
  randomize_registers(cpu);
  cpu.set_register(A0, kArrayAddress);
  cpu.set_register(A1, kArraySize);
  cpu.set_register(RA, 0);
  cpu.set_memory(memory);
  int error = cpu.run_cpu(0, verbose);
  bool error_flag = error != 0;

  if (error_flag) {
    printf("CPU execution error\n");
  }

  for (int i = 0; i < kArraySize - 1; i++) {
    error_flag |= load_wd(mem + kArrayAddress + i * 4) >
                  load_wd(mem + kArrayAddress + i * 4 + 4);
  }

  if (verbose) {
    printf("After:\n");
    for (int i = 0; i < kArraySize; i++) {
      int32_t data = load_wd(mem + kArrayAddress + i * 4);
      printf("%d\t", data);
    }
  }

  if (error_flag) {
    printf("Sort test failed\n");
  }
  return error_flag;
}

bool test_sort_quiet(bool verbose) {
  bool error = test_sort(false);
  if (error & verbose) {
    error = test_sort(true);
  }
  if (verbose) {
    printf("Sort test %s.\n", error ? "failed" : "passed");
  }
  return error;
}
// Sort test ends here.

bool run_test() {

  bool verbose = true;

  bool error = false;
  init_random();

  mem_init();
  error |= test_i_type_loop(verbose);
  error |= test_r_type_loop(verbose);
  error |= test_lui_loop(verbose);
  error |= test_auipc_loop(verbose);
  error |= test_load_loop(verbose);
  error |= test_store_loop(verbose);
  error |= test_b_type_loop(verbose);
  error |= test_jalr_type_loop(verbose);
  error |= test_sum_quiet(verbose);
  error |= test_sort_quiet(verbose);
  // Add test for MRET

  if (error) {
    printf("\nCPU Test failed.\n");
  } else {
    printf("\nAll CPU Tests passed.\n");
  }
  return error;
}

} // namespace cpu_test

int main() {
  return cpu_test::run_test();
}
