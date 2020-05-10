
#include "RISCV_cpu.h"
#include "bit_tools.h"
#include "load_assembler.h"
#include "assembler.h"
#include <map>
#include <random>
#include <iostream>
#include <cassert>

namespace CPUT_TEST {

using namespace RISCV_EMULATOR;

// CPU address bus width.
bool en_64_bit = true;
int xlen;

constexpr int kMemSize = 0x0200000;
std::shared_ptr<MemoryWrapper> memory;

std::mt19937 rnd;
constexpr int kSeed = 155719;

void InitRandom() {
  rnd.seed(kSeed);
}

// The number of test cases for each command.
constexpr int kUnitTestMax = 100;

//  memory initialization
void MemInit() {
  memory = std::make_shared<MemoryWrapper>();
  // mem = memory->data();
}

void RandomizeRegisters(RiscvCpu &cpu) {
  std::mt19937_64 gen(std::random_device{}());

  constexpr int kRegNumber = 32;
  for (int i = 1; i < kRegNumber; i++) {
    uint32_t reg = gen() & 0xFFFFFFFF;
    cpu.SetRegister(i, reg);
  }
}

// Commonly used helper function for error message.
void PrintErrorMessage(const std::string &text, bool error, int64_t expected,
                       int64_t actual) {
  if (error) {
    printf("%s test failed.", text.c_str());
  } else {
    printf("%s test passed.", text.c_str());

  }
  printf(" Expected %016lx, Actual %016lx\n", expected, actual);
  printf(" Expected %ld, Actual %ld\n", expected, actual);
}

// A helper function to split 32 bit GetValue to 20 bit and singed 12 bit immediates.
std::pair<uint32_t, uint32_t> SplitImmediate(uint32_t value) {
  uint32_t value20 = value >> 12;
  uint32_t value12 = value & 0xFFF;
  if (value12 >> 11) {
    value20++;
  }
  // Double check that the sum of these two equals the input.
  if ((value20 << 12) + SignExtend(value12, 12) != value) {
    printf("Test bench error: value1 = %8X, value20 = %8X, value12 = %8X\n",
           value, value20, value12);
  }
  return std::make_pair(value20, value12);
}

// I TYPE test cases starts here.
enum ITYPE_TEST {
  TEST_ADDI,
  TEST_ANDI,
  TEST_ORI,
  TEST_XORI,
  TEST_SLLI,
  TEST_SRLI,
  TEST_SRAI,
  TEST_SLTI,
  TEST_SLTIU,
  TEST_EBREAK,
  TEST_ADDIW,
  TEST_SLLIW,
  TEST_SRAIW,
  TEST_SRLIW
};

bool TestIType(ITYPE_TEST test_type, int32_t rd, int32_t rs1, int32_t value,
               int32_t imm12, bool verbose) {
  if (!en_64_bit &&
      (test_type == TEST_ADDIW || test_type == TEST_SLLIW ||
       test_type == TEST_SRAIW || test_type == TEST_SRLIW)) {
    return false;
  }
  int64_t expected;
  std::string test_case = "";

  // CPU is instantiated here because some tests need access to cpu register.
  RiscvCpu cpu(en_64_bit);
  RandomizeRegisters(cpu);
  auto pointer = memory->begin();
  uint32_t val20, val12;
  std::tie(val20, val12) = SplitImmediate(value);
  AddCmd(pointer, AsmLui(rs1, val20));
  AddCmd(pointer, AsmAddi(rs1, rs1, val12));
  if (rs1 == 0) {
    value = 0;
  }
  uint32_t shift_mask = xlen == 64 ? 0b0111111 : 0b0011111;
  int32_t temp32;
  uint64_t temp64;
  switch (test_type) {
    case TEST_ADDI:
      AddCmd(pointer, AsmAddi(rd, rs1, imm12));
      expected = value + SignExtend(imm12 & 0x0FFF, 12);
      test_case = "ADDI";
      break;
    case TEST_ADDIW:
      AddCmd(pointer, AsmAddiw(rd, rs1, imm12));
      expected = value + SignExtend(imm12 & 0xFFF, 12);
      expected = SignExtend(expected & 0xFFFFFFFF, 32);
      test_case = "ADDIW";
      break;
    case TEST_ANDI:
      AddCmd(pointer, AsmAndi(rd, rs1, imm12));
      expected = value & SignExtend(imm12 & 0x0FFF, 12);
      test_case = "ANDI";
      break;
    case TEST_ORI:
      AddCmd(pointer, AsmOri(rd, rs1, imm12));
      expected = value | SignExtend(imm12 & 0x0FFF, 12);
      test_case = "ORI";
      break;
    case TEST_XORI:
      AddCmd(pointer, AsmXori(rd, rs1, imm12));
      expected = value ^ SignExtend(imm12 & 0x0FFF, 12);
      test_case = "XORI";
      break;
    case TEST_SLLI:
      imm12 = imm12 & shift_mask;
      AddCmd(pointer, AsmSlli(rd, rs1, imm12));
      expected = static_cast<uint64_t>(value) << imm12;
      test_case = "SLLI";
      break;
    case TEST_SLLIW:
      imm12 = imm12 & 0b011111;
      AddCmd(pointer, AsmSlliw(rd, rs1, imm12));
      expected = static_cast<uint64_t>(value) << imm12;
      expected = SignExtend(expected & 0xFFFFFFFF, 32);
      test_case = "SLLIW";
      break;
    case TEST_SRLI:
      imm12 = imm12 & shift_mask;
      AddCmd(pointer, AsmSrli(rd, rs1, imm12));
      temp64 = value;
      if (xlen == 32) {
        temp64 &= 0xFFFFFFFF;
      }
      expected = static_cast<uint64_t>(temp64) >> imm12;
      test_case = "SRLI";
      break;
    case TEST_SRLIW:
      imm12 = imm12 & 0b011111;
      AddCmd(pointer, AsmSrliw(rd, rs1, imm12));
      expected = static_cast<uint64_t>(value & 0xFFFFFFFF) >> imm12;
      expected = SignExtend(expected, 32);
      test_case = "SRLIW";
      break;
    case TEST_SRAI:
      imm12 = imm12 & shift_mask;
      AddCmd(pointer, AsmSrai(rd, rs1, imm12));
      expected = static_cast<int64_t>(value) >> imm12;
      test_case = "SRAI";
      break;
    case TEST_SRAIW:
      imm12 = imm12 & 0b011111;
      AddCmd(pointer, AsmSraiw(rd, rs1, imm12));
      temp32 = value & 0xFFFFFFFF;
      expected = temp32 >> imm12;
      test_case = "SRAIW";
      break;
    case TEST_SLTI:
      AddCmd(pointer, AsmSlti(rd, rs1, imm12));
      expected = value < imm12 ? 1 : 0;
      test_case = "SLTI";
      break;
    case TEST_SLTIU:
      AddCmd(pointer, AsmSltiu(rd, rs1, imm12));
      expected =
        static_cast<uint32_t>(value) < static_cast<uint32_t >(imm12) ? 1 : 0;
      test_case = "SLTIU";
      break;
    case TEST_EBREAK:
      AddCmd(pointer, AsmEbreak());
      expected = cpu.ReadRegister(rd);
      if (rs1 == rd) {
        expected = value;
      }
      test_case = "EBREAK";
      break;
    default:
      printf("I TYPE Test case undefined.\n");
      return true;
  }
  AddCmd(pointer, AsmAddi(A0, rd, 0));
  AddCmd(pointer, AsmXor(RA, RA, RA));
  AddCmd(pointer, AsmJalr(ZERO, RA, 0));

  if (rd == 0) {
    expected = 0;
  }
  if (xlen == 32) {
    expected = SignExtend(expected, 32);
  }
  cpu.SetMemory(memory);
  bool error = cpu.RunCpu(0, verbose) != 0;
  int64_t return_value = cpu.ReadRegister(A0);
  error |= return_value != expected;
  if (error & verbose) {
    printf("RD: %d, RS1: %d, Value: %d(%08x), imm12: %d(%03x)\n", rd, rs1,
           value, value, imm12, imm12);
  }
  if (verbose) {
    PrintErrorMessage(test_case, error, expected, return_value);
  }
  return error;
}

void PrintITypeInstructionMessage(ITYPE_TEST test_case, bool error) {
  std::map<ITYPE_TEST, const std::string> test_name = {{TEST_ADDI,   "ADDI"},
                                                       {TEST_ANDI,   "ANDI"},
                                                       {TEST_ORI,    "ORI"},
                                                       {TEST_XORI,   "XORI"},
                                                       {TEST_SLLI,   "SLLI"},
                                                       {TEST_SRLI,   "SRLI"},
                                                       {TEST_SRAI,   "SRAI"},
                                                       {TEST_SLTI,   "SLTI"},
                                                       {TEST_SLTIU,  "SLTIU"},
                                                       {TEST_EBREAK, "EBREAK"},
                                                       {TEST_ADDIW,  "ADDIW"},
                                                       {TEST_SLLIW,  "SLLIW"},
                                                       {TEST_SRAIW,  "SRAIW"},
                                                       {TEST_SRLIW,  "SRLIW"}
  };
  printf("%s test %s.\n", test_name[test_case].c_str(),
         error ? "failed" : "passed");
}

bool TestITypeLoop(bool verbose) {
  bool total_error = false;
  ITYPE_TEST test_set[] = {TEST_ADDI, TEST_ANDI, TEST_ORI, TEST_XORI, TEST_SLLI,
                           TEST_SRLI, TEST_SRAI, TEST_SLTI,
                           TEST_SLTIU, TEST_EBREAK, TEST_ADDIW, TEST_SLLIW,
                           TEST_SRAIW, TEST_SRLIW};
  for (ITYPE_TEST test_case: test_set) {
    bool error = false;
    for (int i = 0; i < kUnitTestMax && !error; i++) {
      uint32_t rd = rnd() & 0x1F;
      uint32_t rs1 = rnd() & 0x1F;
      int32_t value = rnd();
      int32_t imm12 = SignExtend(rnd() & 0x0FFF, 12);
      bool test_error = TestIType(test_case, rd, rs1, value, imm12, false);
      if (test_error) {
        test_error |= TestIType(test_case, rd, rs1, value, imm12, true);
      }
      error |= test_error;
    }
    if (verbose) {
      PrintITypeInstructionMessage(test_case, error);
    }
    total_error |= error;
  }
  return total_error;
}
// I-Type test cases end here.

// R-Type test cases start here.
enum R_TYPE_TEST {
  TEST_ADD,
  TEST_SUB,
  TEST_AND,
  TEST_OR,
  TEST_XOR,
  TEST_SLL,
  TEST_SRL,
  TEST_SRA,
  TEST_SLT,
  TEST_SLTU,
  TEST_ADDW,
  TEST_SLLW,
  TEST_SRAW,
  TEST_SRLW,
  TEST_SUBW
};

bool
TestRType(R_TYPE_TEST test_type, int32_t rd, int32_t rs1, int32_t rs2,
          int32_t value1, int32_t value2,
          bool verbose) {
  if (!en_64_bit &&
      (test_type == TEST_ADDW || test_type == TEST_SLLW ||
       test_type == TEST_SRAW || test_type == TEST_SRLW ||
       test_type == TEST_SUBW)) {
    return false;
  }

  bool error = false;
  int64_t expected;
  std::string test_case = "";

  auto pointer = memory->begin();
  uint32_t val20, val12;
  std::tie(val20, val12) = SplitImmediate(value1);
  AddCmd(pointer, AsmLui(rs1, val20));
  AddCmd(pointer, AsmAddi(rs1, rs1, val12));
  std::tie(val20, val12) = SplitImmediate(value2);
  AddCmd(pointer, AsmLui(rs2, val20));
  AddCmd(pointer, AsmAddi(rs2, rs2, val12));

  if (rs1 == 0) {
    value1 = 0;
  }
  if (rs2 == 0) {
    value2 = 0;
  }
  if (rs1 == rs2) {
    value1 = value2;
  }
  uint32_t kShiftMask = xlen == 64 ? 0b0111111 : 0b0011111;
  uint64_t temp64;
  switch (test_type) {
    case TEST_ADD:
      AddCmd(pointer, AsmAdd(rd, rs1, rs2));
      expected = static_cast<int64_t>(value1) + static_cast<int64_t >(value2);
      test_case = "ADD";
      break;
    case TEST_ADDW:
      AddCmd(pointer, AsmAddw(rd, rs1, rs2));
      expected =
        (static_cast<int64_t>(value1) + static_cast<int64_t >(value2)) &
        0xFFFFFFFF;
      if (expected >> 31) {
        expected |= 0xFFFFFFFF00000000;
      }
      test_case = "ADDW";
      break;
    case TEST_SUB:
      AddCmd(pointer, AsmSub(rd, rs1, rs2));
      expected = static_cast<int64_t >(value1) - static_cast<int64_t>(value2);
      test_case = "SUB";
      break;
    case TEST_SUBW:
      AddCmd(pointer, AsmSubw(rd, rs1, rs2));
      expected = static_cast<int64_t >(value1) - static_cast<int64_t>(value2);
      expected = SignExtend(expected & 0xFFFFFFFF, 32);
      test_case = "SUBW";
      break;
    case TEST_AND:
      AddCmd(pointer, AsmAnd(rd, rs1, rs2));
      expected = value1 & value2;
      test_case = "AND";
      break;
    case TEST_OR:
      AddCmd(pointer, AsmOr(rd, rs1, rs2));
      expected = value1 | value2;
      test_case = "OR";
      break;
    case TEST_XOR:
      AddCmd(pointer, AsmXor(rd, rs1, rs2));
      expected = value1 ^ value2;
      test_case = "XOR";
      break;
    case TEST_SLL:
      AddCmd(pointer, AsmSll(rd, rs1, rs2));
      expected = static_cast<uint64_t >(value1) << (value2 & kShiftMask);
      test_case = "SLL";
      break;
    case TEST_SLLW:
      AddCmd(pointer, AsmSllw(rd, rs1, rs2));
      expected = static_cast<uint64_t >(value1) << (value2 & 0b011111);
      expected = SignExtend(expected & 0xFFFFFFFF, 32);
      test_case = "SLLW";
      break;
    case TEST_SRL:
      AddCmd(pointer, AsmSrl(rd, rs1, rs2));
      temp64 = value1;
      if (xlen == 32) {
        temp64 &= 0xFFFFFFFF;
      }
      expected = static_cast<uint64_t>(temp64) >> (value2 & kShiftMask);
      test_case = "SRL";
      break;
    case TEST_SRLW:
      AddCmd(pointer, AsmSrlw(rd, rs1, rs2));
      // No sign extension while shifting.
      expected =
        static_cast<uint64_t>(value1 & 0xFFFFFFFF) >> (value2 & 0b011111);
      if (expected >> 31) {
        expected |= 0xFFFFFFFF00000000;
      }
      test_case = "SRLW";
      break;
    case TEST_SRA:
      AddCmd(pointer, AsmSra(rd, rs1, rs2));
      expected = static_cast<int64_t>(value1) >> (value2 & kShiftMask);
      test_case = "SRA";
      break;
    case TEST_SRAW:
      AddCmd(pointer, AsmSraw(rd, rs1, rs2));
      // Do sign extension from 32 bit to 64 bit while shifting.
      expected =
        static_cast<int32_t>(value1 & 0xFFFFFFFF) >> (value2 & 0b011111);
      test_case = "SRAW";
      break;
    case TEST_SLT:
      AddCmd(pointer, AsmSlt(rd, rs1, rs2));
      expected = (value1 < value2) ? 1 : 0;
      test_case = "SLT";
      break;
    case TEST_SLTU:
      AddCmd(pointer, AsmSltu(rd, rs1, rs2));
      expected = (static_cast<uint32_t>(value1) < static_cast<uint32_t>(value2))
                 ? 1 : 0;
      test_case = "SLTU";
      break;
    default:
      if (verbose) {
        printf("Undefined test case.\n");
      }
      return true;
  }
  AddCmd(pointer, AsmAddi(A0, rd, 0));
  AddCmd(pointer, AsmXor(RA, RA, RA));
  AddCmd(pointer, AsmJalr(ZERO, RA, 0));

  if (rd == 0) {
    expected = 0;
  }
  if (xlen == 32) {
    expected = SignExtend(expected, 32);
  }
  RiscvCpu cpu(en_64_bit);
  RandomizeRegisters(cpu);
  cpu.SetMemory(memory);
  error = cpu.RunCpu(0, verbose) != 0;
  int64_t return_value = static_cast<int64_t>(cpu.ReadRegister(A0));
  error |= return_value != expected;
  if (error & verbose) {
    printf("RD: %d, RS1: %d, RS2: %d, Value1: %d(%08x), value2: %d(%03x)\n", rd,
           rs1, rs2, value1, value1,
           value2, value2);
  }
  if (verbose) {
    PrintErrorMessage(test_case, error, expected, return_value);
  }
  return error;
}

void PrintRTypeInstructionMessage(R_TYPE_TEST test_case, bool error) {
  std::map<R_TYPE_TEST, const std::string> test_name = {{TEST_ADD,  "ADD"},
                                                        {TEST_SUB,  "SUB"},
                                                        {TEST_AND,  "AND"},
                                                        {TEST_OR,   "OR"},
                                                        {TEST_XOR,  "XOR"},
                                                        {TEST_SLL,  "SLL"},
                                                        {TEST_SRL,  "SRL"},
                                                        {TEST_SRA,  "SRA"},
                                                        {TEST_SLT,  "SLT"},
                                                        {TEST_SLTU, "SLTU"},
                                                        {TEST_ADDW, "ADDW"},
                                                        {TEST_SLLW, "SLLW"},
                                                        {TEST_SRAW, "SRAW"},
                                                        {TEST_SRLW, "SRLW"},
                                                        {TEST_SUBW, "SUBW"}
  };
  printf("%s test %s.\n", test_name[test_case].c_str(),
         error ? "failed" : "passed");
}

bool TestRTypeLoop(bool verbose = true) {
  bool total_error = false;
  R_TYPE_TEST test_sets[] = {TEST_ADD, TEST_SUB, TEST_AND, TEST_OR, TEST_XOR,
                             TEST_SLL, TEST_SRL, TEST_SRA,
                             TEST_SLT, TEST_SLTU, TEST_ADDW, TEST_SLLW,
                             TEST_SRAW, TEST_SRLW, TEST_SUBW};
  for (R_TYPE_TEST test_case: test_sets) {
    bool error = false;
    for (int i = 0; i < kUnitTestMax && !error; i++) {
      int32_t rd = rnd() & 0x1F;
      int32_t rs1 = rnd() & 0x1F;
      int32_t rs2 = rnd() & 0x1F;
      int32_t value1 = static_cast<int32_t>(rnd());
      int32_t value2 = static_cast<int32_t>(rnd());
      bool test_error = TestRType(test_case, rd, rs1, rs2, value1, value2,
                                  false);
      if (test_error && verbose) {
        test_error = TestRType(test_case, rd, rs1, rs2, value1, value2, true);
      }
      error |= test_error;
    }
    if (verbose) {
      PrintRTypeInstructionMessage(test_case, error);
    }
    total_error |= error;
  }
  return total_error;
}
// R-Type test cases end here.

// AUIPC has it's own test set, starting here.
bool TestAuipc(int32_t rd, int32_t val, int32_t offset, bool verbose) {
  auto pointer = memory->begin();
  AddCmd(pointer, AsmJal(ZERO, offset));
  pointer += offset - 4;
  AddCmd(pointer, AsmAuipc(rd, val));
  AddCmd(pointer, AsmAddi(A0, rd, 0));
  AddCmd(pointer, AsmXor(RA, RA, RA));
  AddCmd(pointer, AsmJalr(ZERO, RA, 0));

  int32_t expected = offset + (val << 12);
  if (rd == 0) {
    expected = 0;
  }
  if (xlen == 32) {
    expected = SignExtend(expected, 32);
  }
  RiscvCpu cpu(en_64_bit);
  RandomizeRegisters(cpu);
  cpu.SetMemory(memory);
  bool error = cpu.RunCpu(0, verbose) != 0;
  int return_value = cpu.ReadRegister(A0);
  error |= return_value != expected;
  if (verbose) {
    PrintErrorMessage("AUIPC", error, expected, return_value);
  }
  return error;
}

bool TestAuipcLoop(bool verbose) {
  bool error = false;
  for (int i = 0; i < kUnitTestMax && !error; i++) {
    int32_t value = static_cast<int32_t>(rnd()) & 0x0FFFFF;
    int32_t offset = static_cast<uint32_t>(rnd()) & 0x0FFF0;
    int32_t rd = rnd() % 32;
    bool test_error = TestAuipc(rd, value, offset, false);
    if (test_error && verbose) {
      test_error |= TestAuipc(rd, value, offset, true);
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
bool TestLui(int32_t val, bool verbose) {
  // LUI test code
  auto pointer = memory->begin();
  AddCmd(pointer, AsmAdd(A0, ZERO, 0));
  AddCmd(pointer, AsmLui(A0, val >> 12));
  AddCmd(pointer, AsmXor(RA, RA, RA));
  AddCmd(pointer, AsmJalr(ZERO, RA, 0));

  int32_t expected = val & 0xFFFFF000;
  expected = SignExtend(expected, 32);
  RiscvCpu cpu(en_64_bit);
  RandomizeRegisters(cpu);
  cpu.SetMemory(memory);
  bool error = cpu.RunCpu(0, verbose) != 0;
  int return_value = cpu.ReadRegister(A0);
  error |= return_value != expected;
  if (verbose) {
    PrintErrorMessage("LUI", error, expected, return_value);
  }
  return error;
}

bool TestLuiLoop(bool verbose) {
  bool error = false;
  for (int i = 0; i < kUnitTestMax && !error; i++) {
    int32_t value = static_cast<int32_t>(rnd());
    bool test_error = TestLui(value, false);
    if (test_error && verbose) {
      test_error = TestLui(value, true);
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
  TEST_LB, TEST_LBU, TEST_LH, TEST_LHU, TEST_LW, TEST_LWU, TEST_LD
};

bool TestLoad(LOAD_TEST test_type, uint32_t rd, uint32_t rs1, uint32_t offset0,
              uint32_t offset1, uint64_t val,
              bool verbose) {
  if (!en_64_bit && (test_type == TEST_LWU || test_type == TEST_LD)) {
    return false;
  }
  bool error = false;
  std::string test_case = "";

  if (rs1 == ZERO) {
    offset0 = 0;
  }
  auto mem = memory->begin();
  uint32_t address = offset0 + SignExtend(offset1, 12);
  for (int i = 0; i < 8; ++i) {
    mem[address + i] = (val >> i * 8) & 0xFF;
  }
  // LW test code
  auto pointer = memory->begin();
  int32_t val20, val12;
  std::tie(val20, val12) = SplitImmediate(offset0);
  int64_t expected;
  AddCmd(pointer, AsmLui(rs1, val20));
  AddCmd(pointer, AsmAddi(rs1, rs1, val12));
  switch (test_type) {
    case TEST_LW:
      AddCmd(pointer, AsmLw(rd, rs1, offset1));
      expected = val & 0xFFFFFFFF;
      expected = SignExtend(expected, 32);
      break;
    case TEST_LWU:
      assert(xlen == 64);
      AddCmd(pointer, AsmLwu(rd, rs1, offset1));
      expected = val & 0xFFFFFFFF;
      break;
    case TEST_LB:
      AddCmd(pointer, AsmLb(rd, rs1, offset1));
      expected = val & 0xFF;
      expected = SignExtend(expected, 8);
      break;
    case TEST_LBU:
      AddCmd(pointer, AsmLbu(rd, rs1, offset1));
      expected = val & 0xFF;
      break;
    case TEST_LH:
      AddCmd(pointer, AsmLh(rd, rs1, offset1));
      expected = val & 0xFFFF;
      expected = SignExtend(expected, 16);
      break;
    case TEST_LHU:
      AddCmd(pointer, AsmLhu(rd, rs1, offset1));
      expected = val & 0xFFFF;
      break;
    case TEST_LD:
      AddCmd(pointer, AsmLd(rd, rs1, offset1));
      expected = val;
      break;
    default:
      if (verbose) {
        printf("Undefined test case %d\n", test_type);
        return true;
      }
  }
  AddCmd(pointer, AsmAddi(A0, rd, 0));
  AddCmd(pointer, AsmXor(RA, RA, RA));
  AddCmd(pointer, AsmJalr(ZERO, RA, 0));
  expected = (rd == ZERO) ? 0 : expected;
  if (xlen == 32) {
    expected = SignExtend(expected, 32);
  }
  RiscvCpu cpu(en_64_bit);
  RandomizeRegisters(cpu);
  cpu.SetMemory(memory);
  error = cpu.RunCpu(0, verbose) != 0;
  int64_t return_value = cpu.ReadRegister(A0);
  error |= return_value != expected;
  if (verbose) {
    PrintErrorMessage(test_case, error, expected, return_value);
    if (error) {
      printf("rd: %2d, rs1: %2d, offset0: %08X, offset1: %08X, val: %08X\n", rd,
             rs1, offset0, offset1, val);
    }
  }
  return error;
}

void PrintLoadInstructionMessage(LOAD_TEST test_case, bool error,
                                 bool verbose = true) {
  if (!verbose) {
    return;
  }
  std::map<LOAD_TEST, const std::string> test_name = {{TEST_LB,  "LB"},
                                                      {TEST_LBU, "LBU"},
                                                      {TEST_LH,  "LH"},
                                                      {TEST_LHU, "LHU"},
                                                      {TEST_LW,  "LW"},
                                                      {TEST_LWU, "LWU"},
                                                      {TEST_LD,  "LD"},
  };
  printf("%s test %s.\n", test_name[test_case].c_str(),
         error ? "failed" : "passed");
}

bool TestLoadLoop(bool verbose) {
  bool error = false;
  LOAD_TEST test_sets[] = {TEST_LB, TEST_LBU, TEST_LH, TEST_LHU, TEST_LW,
                           TEST_LWU, TEST_LD};
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
        offset = offset0_effective + SignExtend(offset1, 12);
      }
      uint64_t val = rnd();
      bool test_error = TestLoad(test_case, rd, rs1, offset0, offset1, val,
                                 false);
      if (test_error && verbose) {
        test_error = TestLoad(test_case, rd, rs1, offset0, offset1, val, true);
      }
      error |= test_error;
    }
    PrintLoadInstructionMessage(test_case, error, verbose);
  }
  return error;
}
// LOAD test cases end here.

// Store test cases start here.
enum STORE_TEST {
  TEST_SW, TEST_SH, TEST_SB, TEST_SD
};

bool
TestStore(STORE_TEST test_type, uint32_t rs1, uint32_t rs2, uint32_t offset0,
          uint32_t offset1, uint32_t value,
          bool verbose) {
  bool error = false;
  std::string test_case = "";
  MemoryWrapper &mem = *memory;

  // STORE test code
  auto pointer = memory->begin();
  uint32_t val20, val12;
  std::tie(val20, val12) = SplitImmediate(value);
  AddCmd(pointer, AsmLui(rs2, val20));
  AddCmd(pointer, AsmAddi(rs2, rs2, val12));
  uint32_t offset20, offset12;
  std::tie(offset20, offset12) = SplitImmediate(offset0);
  AddCmd(pointer, AsmLui(rs1, offset20));
  AddCmd(pointer, AsmAddi(rs1, rs1, offset12));
  AddCmd(pointer, AsmSw(rs1, rs2, offset1));
  int64_t expected;
  switch (test_type) {
    case TEST_SW:
      test_case = "SW";
      AddCmd(pointer, AsmSw(rs1, rs2, offset1));
      expected = value & 0xFFFFFFFF;
      break;
    case TEST_SH:
      test_case = "SH";
      AddCmd(pointer, AsmSh(rs1, rs2, offset1));
      expected = value & 0x0000FFFF;
      break;
    case TEST_SB:
      test_case = "SB";
      AddCmd(pointer, AsmSb(rs1, rs2, offset1));
      expected = value & 0x000000FF;
      break;
    case TEST_SD:
      test_case = "SD";
      AddCmd(pointer, AsmSd(rs1, rs2, offset1));
      expected = SignExtend(static_cast<uint64_t >(value), 32);
      break;
    default:
      if (verbose) {
        printf("Undefined test case %d\n", test_type);
      }
      return true;
  }
  AddCmd(pointer, AsmAddi(A0, ZERO, 0));
  AddCmd(pointer, AsmXor(RA, RA, RA));
  AddCmd(pointer, AsmJalr(ZERO, RA, 0));
  expected = (rs2 == ZERO) ? 0 : expected;

  RiscvCpu cpu(en_64_bit);
  RandomizeRegisters(cpu);
  cpu.SetMemory(memory);
  error = cpu.RunCpu(0, verbose) != 0;
  offset0 = (rs1 == ZERO) ? 0 : offset0;
  uint32_t address = offset0 + SignExtend(offset1, 12);
  int size =
    test_type == TEST_SH ? 2 : (test_type == TEST_SW ? 4 : (test_type == TEST_SD
                                                            ? 8 : 1));
  uint64_t result = 0;
  for (int i = 0; i < size; ++i) {
    result |= static_cast<uint64_t>(mem[address + i]) << (8 * i);
  }
  error |= result != expected;
  if (verbose) {
    PrintErrorMessage(test_case, error, expected, result);
    if (error) {
      printf("rs1: %2d, rs2: %2d, offset0: %08X, offset1: %08X, val: %08X\n",
             rs1, rs2, offset0, offset1,
             value);
    }
  }
  return error;
}

void PrintStoreInstructionMessage(STORE_TEST test_case, bool error,
                                  bool verbose = true) {
  if (!verbose) {
    return;
  }
  std::map<STORE_TEST, const std::string> test_name = {{TEST_SW, "SW"},
                                                       {TEST_SH, "SH"},
                                                       {TEST_SB, "SB"},
                                                       {TEST_SD, "SD"},
  };
  printf("%s test %s.\n", test_name[test_case].c_str(),
         error ? "failed" : "passed");
}

bool TestStoreLoop(bool verbose) {
  bool error = false;
  STORE_TEST test_sets[] = {TEST_SW, TEST_SH, TEST_SB, TEST_SD};
  for (auto test_case: test_sets) {
    for (int i = 0; i < kUnitTestMax && !error; i++) {
      int32_t rs1 = rnd() % 32;
      int32_t rs2 = rnd() % 32;
      uint32_t offset0 = 0, offset1 = 0, offset = 0;
      while (offset < 40 || offset >= kMemSize - 4) {
        int32_t offset0_effective;
        offset0 = rnd() % kMemSize;
        offset1 = rnd() & 0x0FFF;
        offset0_effective = (rs1 == ZERO) ? 0 : offset0;
        offset = offset0_effective + SignExtend(offset1, 12);
      }
      int32_t value = rnd() & 0xFFFFFFFF;
      if (rs1 == rs2) {
        value = offset0;
      }
      bool test_error = TestStore(test_case, rs1, rs2, offset0, offset1, value,
                                  false);
      if (test_error && verbose) {
        test_error = TestStore(test_case, rs1, rs2, offset0, offset1, value,
                               true);
      }
      error |= test_error;
    }
    PrintStoreInstructionMessage(test_case, error, verbose);
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

void PrintBTypeInstructionMessage(B_TYPE_TEST test_case, bool error) {
  printf("%s test %s.\n", test_name[test_case].c_str(),
         error ? "failed" : "passed");
}

bool
TestBType(B_TYPE_TEST test_type, uint32_t rs1, uint32_t rs2, uint32_t value1,
          uint32_t value2, int32_t offset,
          bool verbose = true) {
  bool error = false;
  std::string test_case = test_name[test_type];

  value1 = (rs1 == ZERO) ? 0 : value1;
  value2 = (rs2 == ZERO) ? 0 : value2;
  value1 = (rs1 == rs2) ? value2 : value1;

  uint32_t start_point = kMemSize / 2;
  MemorWrapperIterator pointer = memory->begin() + start_point;
  uint32_t value20, value12;
  uint32_t expected;
  std::tie(value20, value12) = SplitImmediate(value1);
  AddCmd(pointer, AsmLui(rs1, value20));
  AddCmd(pointer, AsmAddi(rs1, rs1, value12));
  std::tie(value20, value12) = SplitImmediate(value2);
  AddCmd(pointer, AsmLui(rs2, value20));
  AddCmd(pointer, AsmAddi(rs2, rs2, value12));
  MemorWrapperIterator next_pos = pointer + offset;
  if (test_type == TEST_BEQ) {
    AddCmd(pointer, AsmBeq(rs1, rs2, offset));
    expected = (value1 == value2) ? 1 : 0;
  } else if (test_type == TEST_BGE) {
    AddCmd(pointer, AsmBge(rs1, rs2, offset));
    expected = (static_cast<int32_t>(value1) >= static_cast<int32_t>(value2))
               ? 1 : 0;
  } else if (test_type == TEST_BGEU) {
    AddCmd(pointer, AsmBgeu(rs1, rs2, offset));
    expected = value1 >= value2 ? 1 : 0;
  } else if (test_type == TEST_BLT) {
    AddCmd(pointer, AsmBlt(rs1, rs2, offset));
    expected =
      static_cast<int32_t>(value1) < static_cast<int32_t>(value2) ? 1 : 0;
  } else if (test_type == TEST_BLTU) {
    AddCmd(pointer, AsmBltu(rs1, rs2, offset));
    expected = value1 < value2 ? 1 : 0;
  } else if (test_type == TEST_BNE) {
    AddCmd(pointer, AsmBne(rs1, rs2, offset));
    expected = value1 != value2 ? 1 : 0;
  }
  AddCmd(pointer, AsmAddi(A0, ZERO, 0));
  AddCmd(pointer, AsmXor(RA, RA, RA));
  AddCmd(pointer, AsmJalr(ZERO, RA, 0));
  pointer = next_pos;
  AddCmd(pointer, AsmAddi(A0, ZERO, 1));
  AddCmd(pointer, AsmXor(RA, RA, RA));
  AddCmd(pointer, AsmJalr(ZERO, RA, 0));


  RiscvCpu cpu(en_64_bit);
  RandomizeRegisters(cpu);
  cpu.SetMemory(memory);
  error = cpu.RunCpu(start_point, verbose) != 0;
  int64_t return_value = static_cast<int64_t>(cpu.ReadRegister(A0));
  error |= return_value != expected;
  if (error & verbose) {
    printf(
      "RS1: %d, RS2: %d, value1: %d(%08x), value2: %d(%08x), offset: %d(%03x)\n",
      rs1, rs2, value1, value1, value2, value2, offset, offset);
  }

  if (verbose) {
    PrintErrorMessage(test_case, error, expected, return_value);
  }

  return error;
}

bool TestBTypeLoop(bool verbose = true) {
  bool total_error = false;
  B_TYPE_TEST test_sets[] = {TEST_BEQ, TEST_BGE, TEST_BGEU, TEST_BLT, TEST_BLTU,
                             TEST_BNE};

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
          // You can ignore the corner case new random GetValue equals to value1.
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
      bool test_error = TestBType(test_case, rs1, rs2, value1, value2, offset,
                                  false);
      if (test_error) {
        test_error = TestBType(test_case, rs1, rs2, value1, value2, offset,
                               true);
      }
      error |= test_error;
    }
    total_error |= error;
    if (verbose) {
      PrintBTypeInstructionMessage(test_case, error);
    }
  }
  return total_error;
}
// B-Type tests end here.

void PrintJalrTypeInstructionMessage(bool error) {
  printf("%s test %s.\n", "JALR", error ? "failed" : "passed");
}

bool TestJalrType(uint32_t rd, uint32_t rs1, uint32_t offset, uint32_t value,
                  bool verbose) {
  std::string test_case = "JALR";

  if (rs1 == 0) {
    value = 0;
  }
  uint32_t start_point = kMemSize / 4;
  auto pointer = memory->begin() + start_point;

  uint32_t value20, value12;
  std::tie(value20, value12) = SplitImmediate(value);
  AddCmd(pointer, AsmLui(rs1, value20));
  AddCmd(pointer, AsmAddi(rs1, rs1, value12));
  AddCmd(pointer, AsmJalr(rd, rs1, offset));
  AddCmd(pointer, AsmAddi(A0, ZERO, 1));
  AddCmd(pointer, AsmXor(RA, RA, RA));
  AddCmd(pointer, AsmJalr(ZERO, RA, 0));
  pointer = memory->begin() + ((value + SignExtend(offset, 12)) & ~1);
  AddCmd(pointer, AsmAddi(A0, ZERO, 2));
  AddCmd(pointer, AsmXor(RA, RA, RA));
  AddCmd(pointer, AsmJalr(ZERO, RA, 0));
  uint32_t expected = 2;
  RiscvCpu cpu(en_64_bit);
  RandomizeRegisters(cpu);
  cpu.SetMemory(memory);
  bool error = cpu.RunCpu(start_point, verbose) != 0;
  int64_t return_value = static_cast<int64_t>(cpu.ReadRegister(A0));
  error |= return_value != expected;
  if (rd != 0 && rd != RA && rd != A0) {
    uint32_t expect = start_point + 12;
    uint32_t actual = cpu.ReadRegister(rd);
    error |= actual != expect;
    if (actual != expect) {
      printf("reg[rd] = %d(%08x), expected = %d(%08x)\n", actual, actual,
             expect, expect);
    }
  }
  if (error & verbose) {
    printf("RS1: %d, RD: %d, GetValue: %d(%08x), offset: %d(%03x)\n",
           rs1, rd, value, value, offset, offset);
  }

  if (verbose) {
    PrintErrorMessage(test_case, error, expected, return_value);
  }

  return error;
}

bool TestJalrTypeLoop(bool verbose = true) {
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
    bool test_error = TestJalrType(rd, rs1, offset, value, false);
    if (test_error) {
      test_error = TestJalrType(rd, rs1, offset, value, true);
    }
    error |= test_error;
  }
  if (verbose) {
    PrintJalrTypeInstructionMessage(error);
  }
  return error;
}

// Summation test starts here.
bool TestSum(bool verbose) {
  auto pointer = memory->begin();
  LoadAssemblerSum(pointer);
  constexpr int kExpectedValue = 55;
  RiscvCpu cpu(en_64_bit);
  RandomizeRegisters(cpu);
  cpu.SetMemory(memory);
  bool error = cpu.RunCpu(0, verbose) != 0;
  int return_value = cpu.ReadRegister(A0);
  error |= return_value != kExpectedValue;
  if (verbose) {
    PrintErrorMessage("Summation", error, kExpectedValue, return_value);
  }
  return error;
}

bool TestSumQuiet(bool verbose) {
  bool error = TestSum(false);
  if (error & verbose) {
    error = TestSum(true);
  }
  if (verbose) {
    printf("SUM test %s.\n", error ? "failed" : "passed");
  }
  return error;
}
// Summation test ends here.

// Sort test starts here.
bool TestSort(bool verbose) {
  auto assembly_pointer = memory->begin();
  LoadAssemblerSort(assembly_pointer);

  constexpr int kArraySize = 100;
  constexpr int kArrayAddress = 512;
  auto value_pointer = memory->begin() + kArrayAddress;
  for (int i = 0; i < kArraySize * 4; i++) {
    int value = static_cast<int>(rnd() % 1000);
    StoreWd(value_pointer + 4 * i, value);
  }

  if (verbose) {
    std::cout << "Before:\n";
    for (int i = 0; i < kArraySize; i++) {
      std::cout << LoadWd(value_pointer + i * 4) << "\t";
    }
    std::cout << std::endl;
  }

  RiscvCpu cpu(en_64_bit);
  RandomizeRegisters(cpu);
  cpu.SetRegister(A0, kArrayAddress);
  cpu.SetRegister(A1, kArraySize);
  cpu.SetRegister(RA, 0);
  cpu.SetMemory(memory);
  int error = cpu.RunCpu(0, verbose);
  bool error_flag = error != 0;

  if (error_flag) {
    printf("CPU execution error\n");
  }

  for (int i = 0; i < kArraySize - 1; i++) {
    error_flag |= LoadWd(value_pointer + i * 4) >
                  LoadWd(value_pointer + i * 4 + 4);
  }

  if (verbose) {
    printf("After:\n");
    for (int i = 0; i < kArraySize; i++) {
      int32_t data = LoadWd(value_pointer + i * 4);
      printf("%d\t", data);
    }
  }

  if (error_flag) {
    printf("Sort test failed\n");
  }
  return error_flag;
}

bool TestSortQuiet(bool verbose) {
  bool error = TestSort(false);
  if (error & verbose) {
    error = TestSort(true);
  }
  if (verbose) {
    printf("Sort test %s.\n", error ? "failed" : "passed");
  }
  return error;
}
// Sort test ends here.

bool RunTest() {

  // CPU address bus width.

  bool verbose = true;
  bool error = false;
  InitRandom();

  MemInit();
  for (int i = 0; i < 2; i++) {
    en_64_bit = i == 0 ? false : true;
    if (en_64_bit) {
      xlen = 64;
      std::cout << "64bit test start." << std::endl;
    } else {
      xlen = 32;
      std::cout << "32bit test start." << std::endl;
    }
    error |= TestITypeLoop(verbose);
    error |= TestRTypeLoop(verbose);
    error |= TestLuiLoop(verbose);
    error |= TestAuipcLoop(verbose);
    error |= TestLoadLoop(verbose);
    error |= TestStoreLoop(verbose);
    error |= TestBTypeLoop(verbose);
    error |= TestJalrTypeLoop(verbose);
    error |= TestSumQuiet(verbose);
    error |= TestSortQuiet(verbose);
    // Add test for MRET
  }

  if (error) {
    printf("\nCPU Test failed.\n");
  } else {
    printf("\nAll CPU Tests passed.\n");
  }
  return error;
}

} // namespace cpu_test

int main() {
  return CPUT_TEST::RunTest();
}
