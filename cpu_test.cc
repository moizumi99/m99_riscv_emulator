#include "RISCV_cpu.h"
#include "bit_tools.h"
#include "load_assembler.h"
#include "assembler.h"
#include <map>
#include <random>
#include <iostream>
#include <cassert>

using namespace RISCV_EMULATOR;
using namespace CPU_TEST;

namespace {
// CPU address bus width.
bool en_ctest = true;
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
  TEST_ADDI, TEST_ANDI, TEST_ORI, TEST_XORI,
  TEST_SLLI, TEST_SRLI, TEST_SRAI,
  TEST_SLTI, TEST_SLTIU, TEST_EBREAK,
  TEST_ADDIW, TEST_SLLIW, TEST_SRAIW, TEST_SRLIW,
  TEST_CADDI, TEST_CADDIW, TEST_CADDI16SP, TEST_CADDI4SPN,
  TEST_CANDI, TEST_CLI, TEST_CLUI,
  TEST_CSRAI, TEST_CSRLI, TEST_CSLLI,
  TEST_CEBREAK,
};

bool TestIType(ITYPE_TEST test_type, int32_t rd, int32_t rs1, int32_t value,
               int32_t imm, bool verbose) {
  if (!en_64_bit &&
      (test_type == TEST_ADDIW || test_type == TEST_SLLIW ||
       test_type == TEST_SRAIW || test_type == TEST_SRLIW ||
       test_type == TEST_CADDIW)) {
    return false;
  }
  if (!en_ctest && (test_type == TEST_CADDI || test_type == TEST_CADDI16SP ||
                    test_type == TEST_CADDIW || test_type == TEST_CANDI ||
                    test_type == TEST_CSLLI ||
                    test_type == TEST_CSRAI || test_type == TEST_CSRLI ||
                    test_type == TEST_CEBREAK)) {
    return false;
  }
  if (test_type == TEST_CADDI16SP) {
    rs1 = 2;
    imm = SignExtend(imm & 0b1111100000, 12);
    if (imm == 0) {
      return false;
    }
  } else if (test_type == TEST_CADDI4SPN) {
    rs1 = 2;
    rd = (rd & 0b111) + 8;
    imm &= 0b1111111100;
    if (imm == 0) {
      return false;
    }
  } else if (test_type == TEST_CANDI) {
    rd = rs1 = (rs1 & 0b111) + 8;
    imm &= 0b0111111;
  } else if (test_type == TEST_CLI) {
    rs1 = 0;
    imm = SignExtend(imm, 6);
  } else if (test_type == TEST_CLUI) {
    rd = rs1 = 0;
    imm = SignExtend(imm << 12, 18);
    if (imm == 0) {
      return false;
    }
  } else if (test_type == TEST_CSRAI || test_type == TEST_CSRLI) {
    rs1 = rd = (rd & 0b111) + 8;
    imm = en_64_bit ? (imm & 0b0111111) : (imm &
                                           0b011111); // imm is unsigned here.
  } else if (test_type == TEST_CSLLI) {
    rs1 = rd;
    imm &= 0b111111;
  }


  int64_t expected;
  std::string test_case = "";

  // CPU is instantiated here because some tests need access to cpu register.
  RiscvCpu cpu(en_64_bit);
  RandomizeRegisters(cpu);
  uint64_t pointer = 0;
  uint32_t val20, val12;
  std::tie(val20, val12) = SplitImmediate(value);
  pointer = AddCmd(*memory, pointer, AsmLui(rs1, val20));
  pointer = AddCmd(*memory, pointer, AsmAddi(rs1, rs1, val12));
  if (rs1 == 0) {
    value = 0;
  }
  uint32_t shift_mask = xlen == 64 ? 0b0111111 : 0b0011111;
  int32_t temp32;
  uint64_t temp64;
  switch (test_type) {
    case TEST_ADDI:
      pointer = AddCmd(*memory, pointer, AsmAddi(rd, rs1, imm));
      expected = value + SignExtend(imm & 0x0FFF, 12);
      test_case = "ADDI";
      break;
    case TEST_ADDIW:
      pointer = AddCmd(*memory, pointer, AsmAddiw(rd, rs1, imm));
      expected = value + SignExtend(imm & 0xFFF, 12);
      expected = SignExtend(expected & 0xFFFFFFFF, 32);
      test_case = "ADDIW";
      break;
    case TEST_ANDI:
      pointer = AddCmd(*memory, pointer, AsmAndi(rd, rs1, imm));
      expected = value & SignExtend(imm & 0x0FFF, 12);
      test_case = "ANDI";
      break;
    case TEST_ORI:
      pointer = AddCmd(*memory, pointer, AsmOri(rd, rs1, imm));
      expected = value | SignExtend(imm & 0x0FFF, 12);
      test_case = "ORI";
      break;
    case TEST_XORI:
      pointer = AddCmd(*memory, pointer, AsmXori(rd, rs1, imm));
      expected = value ^ SignExtend(imm & 0x0FFF, 12);
      test_case = "XORI";
      break;
    case TEST_SLLI:
      imm = imm & shift_mask;
      pointer = AddCmd(*memory, pointer, AsmSlli(rd, rs1, imm));
      expected = static_cast<uint64_t>(value) << imm;
      test_case = "SLLI";
      break;
    case TEST_SLLIW:
      imm = imm & 0b011111;
      pointer = AddCmd(*memory, pointer, AsmSlliw(rd, rs1, imm));
      expected = static_cast<uint64_t>(value) << imm;
      expected = SignExtend(expected & 0xFFFFFFFF, 32);
      test_case = "SLLIW";
      break;
    case TEST_SRLI:
      imm = imm & shift_mask;
      pointer = AddCmd(*memory, pointer, AsmSrli(rd, rs1, imm));
      temp64 = value;
      if (xlen == 32) {
        temp64 &= 0xFFFFFFFF;
      }
      expected = static_cast<uint64_t>(temp64) >> imm;
      test_case = "SRLI";
      break;
    case TEST_SRLIW:
      imm = imm & 0b011111;
      pointer = AddCmd(*memory, pointer, AsmSrliw(rd, rs1, imm));
      expected = static_cast<uint64_t>(value & 0xFFFFFFFF) >> imm;
      expected = SignExtend(expected, 32);
      test_case = "SRLIW";
      break;
    case TEST_SRAI:
      imm = imm & shift_mask;
      pointer = AddCmd(*memory, pointer, AsmSrai(rd, rs1, imm));
      expected = static_cast<int64_t>(value) >> imm;
      test_case = "SRAI";
      break;
    case TEST_SRAIW:
      imm = imm & 0b011111;
      pointer = AddCmd(*memory, pointer, AsmSraiw(rd, rs1, imm));
      temp32 = value & 0xFFFFFFFF;
      expected = temp32 >> imm;
      test_case = "SRAIW";
      break;
    case TEST_SLTI:
      pointer = AddCmd(*memory, pointer, AsmSlti(rd, rs1, imm));
      expected = value < imm ? 1 : 0;
      test_case = "SLTI";
      break;
    case TEST_SLTIU:
      pointer = AddCmd(*memory, pointer, AsmSltiu(rd, rs1, imm));
      expected =
        static_cast<uint32_t>(value) < static_cast<uint32_t >(imm) ? 1 : 0;
      test_case = "SLTIU";
      break;
    case TEST_EBREAK:
      pointer = AddCmd(*memory, pointer, AsmEbreak());
      expected = cpu.ReadRegister(rd);
      if (rs1 == rd) {
        expected = value;
      }
      test_case = "EBREAK";
      break;
    case TEST_CADDI:
    case TEST_CADDIW:
      imm = SignExtend(imm, 6);
      rd = rs1;
      if (test_type == TEST_CADDI) {
        pointer = AddCmdCType(*memory, pointer, AsmCAddi(rd, imm));
      } else {
        pointer = AddCmdCType(*memory, pointer, AsmCAddiw(rd, imm));
      }
      expected = value + imm;
      test_case = test_type == TEST_CADDI ? "C.ADDI" : "C.ADDIW";
      break;
    case TEST_CADDI16SP:
      imm = SignExtend(imm & 0b1111100000, 10);
      rd = rs1;
      pointer = AddCmdCType(*memory, pointer, AsmCAddi16sp(imm));
      expected = value + imm;
      test_case = "C.ADDI16SP";
      break;
    case TEST_CADDI4SPN:
      pointer = AddCmdCType(*memory, pointer, AsmCAddi4spn(rd, imm));
      expected = value + imm;
      test_case = "C.ADDI4SPN";
      break;
    case TEST_CANDI:
      pointer = AddCmdCType(*memory, pointer, AsmCAndi(rd, imm));
      expected = value & SignExtend(imm, 6);
      test_case = "C.ANDI";
      break;
    case TEST_CLI:
      pointer = AddCmdCType(*memory, pointer, AsmCLi(rd, imm));
      expected = SignExtend(imm, 6);
      test_case = "C.LI";
      break;
    case TEST_CLUI:
      pointer = AddCmdCType(*memory, pointer, AsmCLui(rd, imm));
      expected = SignExtend(imm, 18);
      test_case = "C.LUI";
      break;
    case TEST_CSLLI:
      imm = imm & shift_mask;
      pointer = AddCmdCType(*memory, pointer, AsmCSlli(rd, imm));
      expected = static_cast<uint64_t>(value) << imm;
      test_case = "C.SLLI";
      break;
    case TEST_CSRAI:
      pointer = AddCmdCType(*memory, pointer, AsmCSrai(rd, imm));
      expected = static_cast<int64_t>(value) >> imm;
      test_case = "C.SRAI";
      break;
    case TEST_CSRLI:
      pointer = AddCmdCType(*memory, pointer, AsmCSrli(rd, imm));
      temp64 = value;
      if (xlen == 32) {
        temp64 &= 0xFFFFFFFF;
      }
      expected = static_cast<uint64_t>(temp64) >> imm;
      test_case = "C.SRLI";
      break;
    case TEST_CEBREAK:
      pointer = AddCmdCType(*memory, pointer, AsmCEbreak());
      expected = cpu.ReadRegister(rd);
      if (rs1 == rd) {
        expected = value;
      }
      test_case = "C.EBREAK";
      break;
    default:
      printf("I TYPE Test case undefined.\n");
      return true;
  }
  pointer = AddCmd(*memory, pointer, AsmAddi(A0, rd, 0));
  pointer = AddCmd(*memory, pointer, AsmXor(RA, RA, RA));
  pointer = AddCmd(*memory, pointer, AsmJalr(ZERO, RA, 0));

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
           value, value, imm, imm);
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
                                                       {TEST_SRLIW,  "SRLIW"},
                                                       {TEST_CADDI,  "C.ADDI"},
                                                       {TEST_CADDIW, "C.ADDIW"},
                                                       {TEST_CADDI16SP,
                                                                     "C.ADDI16SP"},
                                                       {TEST_CADDI4SPN,
                                                                     "C.ADDI4SPN"},
                                                       {TEST_CANDI,  "C.ANDI"},
                                                       {TEST_CLI,    "C.LI"},
                                                       {TEST_CLUI,   "C.LUI"},
                                                       {TEST_CSLLI,  "C.SLLI"},
                                                       {TEST_CSRAI,  "C.SRAI"},
                                                       {TEST_CSRLI,  "C.SRLI"},
                                                       {TEST_CEBREAK,
                                                                     "C.EBREAK"},
  };
  printf("%s test %s.\n", test_name[test_case].c_str(),
         error ? "failed" : "passed");
}

bool TestITypeLoop(bool verbose) {
  bool total_error = false;
  ITYPE_TEST test_set[] = {TEST_ADDI, TEST_ANDI, TEST_ORI, TEST_XORI, TEST_SLLI,
                           TEST_SRLI, TEST_SRAI, TEST_SLTI,
                           TEST_SLTIU, TEST_EBREAK, TEST_ADDIW, TEST_SLLIW,
                           TEST_SRAIW, TEST_SRLIW, TEST_CADDI, TEST_CADDIW,
                           TEST_CADDI16SP, TEST_CADDI4SPN, TEST_CANDI, TEST_CLI,
                           TEST_CLUI, TEST_CSLLI, TEST_CSRAI, TEST_CSRLI,
                           TEST_CEBREAK,
  };
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
  TEST_ADD, TEST_SUB, TEST_AND, TEST_OR, TEST_XOR,
  TEST_SLL, TEST_SRL, TEST_SRA, TEST_SLT, TEST_SLTU,
  TEST_ADDW, TEST_SLLW, TEST_SRAW, TEST_SRLW, TEST_SUBW,
  TEST_CADD, TEST_CAND, TEST_CADDW, TEST_COR,
  TEST_CSUB, TEST_CSUBW, TEST_CXOR, TEST_CMV,
};

bool
TestRType(R_TYPE_TEST test_type, int32_t rd, int32_t rs1, int32_t rs2,
          int32_t value1, int32_t value2,
          bool verbose) {
  if (!en_64_bit &&
      (test_type == TEST_ADDW || test_type == TEST_SLLW ||
       test_type == TEST_SRAW || test_type == TEST_SRLW ||
       test_type == TEST_SUBW || test_type == TEST_CADDW ||
       test_type == TEST_CSUBW)) {
    return false;
  }
  if (!en_ctest && (test_type == TEST_CADD || test_type == TEST_CAND ||
                    test_type == TEST_CADDW || test_type == TEST_COR ||
                    test_type == TEST_CSUB
                    || test_type == TEST_CSUBW || test_type == TEST_CXOR ||
                    test_type == TEST_CMV)) {
    return false;
  }

  if (test_type == TEST_CAND || test_type == TEST_CADDW || test_type == TEST_COR
      || test_type == TEST_CSUB || test_type == TEST_CSUBW ||
      test_type == TEST_CXOR) {
    rd = rs1 = (rs1 & 0b111) + 8;
    rs2 = (rs2 & 0b111) + 8;
  }

  bool error = false;
  int64_t expected;
  std::string test_case = "";

  uint64_t pointer = 0;
  uint32_t val20, val12;
  std::tie(val20, val12) = SplitImmediate(value1);
  pointer = AddCmd(*memory, pointer, AsmLui(rs1, val20));
  pointer = AddCmd(*memory, pointer, AsmAddi(rs1, rs1, val12));
  std::tie(val20, val12) = SplitImmediate(value2);
  pointer = AddCmd(*memory, pointer, AsmLui(rs2, val20));
  pointer = AddCmd(*memory, pointer, AsmAddi(rs2, rs2, val12));

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
      pointer = AddCmd(*memory, pointer, AsmAdd(rd, rs1, rs2));
      expected = static_cast<int64_t>(value1) + static_cast<int64_t >(value2);
      test_case = "ADD";
      break;
    case TEST_ADDW:
      pointer = AddCmd(*memory, pointer, AsmAddw(rd, rs1, rs2));
      expected =
        (static_cast<int64_t>(value1) + static_cast<int64_t >(value2)) &
        0xFFFFFFFF;
      if (expected >> 31) {
        expected |= 0xFFFFFFFF00000000;
      }
      test_case = "ADDW";
      break;
    case TEST_SUB:
      pointer = AddCmd(*memory, pointer, AsmSub(rd, rs1, rs2));
      expected = static_cast<int64_t >(value1) - static_cast<int64_t>(value2);
      test_case = "SUB";
      break;
    case TEST_SUBW:
      pointer = AddCmd(*memory, pointer, AsmSubw(rd, rs1, rs2));
      expected = static_cast<int64_t >(value1) - static_cast<int64_t>(value2);
      expected = SignExtend(expected & 0xFFFFFFFF, 32);
      test_case = "SUBW";
      break;
    case TEST_AND:
      pointer = AddCmd(*memory, pointer, AsmAnd(rd, rs1, rs2));
      expected = value1 & value2;
      test_case = "AND";
      break;
    case TEST_OR:
      pointer = AddCmd(*memory, pointer, AsmOr(rd, rs1, rs2));
      expected = value1 | value2;
      test_case = "OR";
      break;
    case TEST_XOR:
      pointer = AddCmd(*memory, pointer, AsmXor(rd, rs1, rs2));
      expected = value1 ^ value2;
      test_case = "XOR";
      break;
    case TEST_SLL:
      pointer = AddCmd(*memory, pointer, AsmSll(rd, rs1, rs2));
      expected = static_cast<uint64_t >(value1) << (value2 & kShiftMask);
      test_case = "SLL";
      break;
    case TEST_SLLW:
      pointer = AddCmd(*memory, pointer, AsmSllw(rd, rs1, rs2));
      expected = static_cast<uint64_t >(value1) << (value2 & 0b011111);
      expected = SignExtend(expected & 0xFFFFFFFF, 32);
      test_case = "SLLW";
      break;
    case TEST_SRL:
      pointer = AddCmd(*memory, pointer, AsmSrl(rd, rs1, rs2));
      temp64 = value1;
      if (xlen == 32) {
        temp64 &= 0xFFFFFFFF;
      }
      expected = static_cast<uint64_t>(temp64) >> (value2 & kShiftMask);
      test_case = "SRL";
      break;
    case TEST_SRLW:
      pointer = AddCmd(*memory, pointer, AsmSrlw(rd, rs1, rs2));
      // No sign extension while shifting.
      expected =
        static_cast<uint64_t>(value1 & 0xFFFFFFFF) >> (value2 & 0b011111);
      if (expected >> 31) {
        expected |= 0xFFFFFFFF00000000;
      }
      test_case = "SRLW";
      break;
    case TEST_SRA:
      pointer = AddCmd(*memory, pointer, AsmSra(rd, rs1, rs2));
      expected = static_cast<int64_t>(value1) >> (value2 & kShiftMask);
      test_case = "SRA";
      break;
    case TEST_SRAW:
      pointer = AddCmd(*memory, pointer, AsmSraw(rd, rs1, rs2));
      // Do sign extension from 32 bit to 64 bit while shifting.
      expected =
        static_cast<int32_t>(value1 & 0xFFFFFFFF) >> (value2 & 0b011111);
      test_case = "SRAW";
      break;
    case TEST_SLT:
      pointer = AddCmd(*memory, pointer, AsmSlt(rd, rs1, rs2));
      expected = (value1 < value2) ? 1 : 0;
      test_case = "SLT";
      break;
    case TEST_SLTU:
      pointer = AddCmd(*memory, pointer, AsmSltu(rd, rs1, rs2));
      expected = (static_cast<uint32_t>(value1) < static_cast<uint32_t>(value2))
                 ? 1 : 0;
      test_case = "SLTU";
      break;
    case TEST_CADD:
      if (rs1 == 0 || rs2 == 0) {
        // if rd == 0 || rs2 == 0, c.add is invalid.
        return false;
      }
      rd = rs1;
      pointer = AddCmdCType(*memory, pointer, AsmCAdd(rd, rs2));
      expected = static_cast<int64_t>(value1) + static_cast<int64_t >(value2);
      test_case = "C.ADD";
      break;
    case TEST_CAND:
      pointer = AddCmdCType(*memory, pointer, AsmCAnd(rd, rs2));
      expected = value1 & value2;
      test_case = "C.AND";
      break;
    case TEST_CADDW:
      pointer = AddCmdCType(*memory, pointer, AsmCAddw(rd, rs2));
      expected =
        (static_cast<int64_t>(value1) + static_cast<int64_t >(value2)) &
        0xFFFFFFFF;
      expected = SignExtend(expected, 32);
      test_case = "CADDW";
      break;
    case TEST_COR:
      pointer = AddCmdCType(*memory, pointer, AsmCOr(rd, rs2));
      expected = value1 | value2;
      test_case = "C.OR";
      break;
    case TEST_CSUB:
      pointer = AddCmdCType(*memory, pointer, AsmCSub(rd, rs2));
      expected = static_cast<int64_t >(value1) - static_cast<int64_t>(value2);
      test_case = "C.SUB";
      break;
    case TEST_CSUBW:
      pointer = AddCmdCType(*memory, pointer, AsmCSubw(rd, rs2));
      expected = static_cast<int64_t >(value1) - static_cast<int64_t>(value2);
      expected = SignExtend(expected & 0xFFFFFFFF, 32);
      test_case = "C.SUBW";
      break;
    case TEST_CXOR:
      pointer = AddCmdCType(*memory, pointer, AsmCXor(rd, rs2));
      expected = value1 ^ value2;
      test_case = "C.XOR";
      break;
    case TEST_CMV:
      pointer = AddCmdCType(*memory, pointer, AsmCMv(rd, rs2));
      expected = static_cast<int64_t >(value2);
      test_case = "C.MV";
      if (rs2 == 0) {
        return false;
      }
      break;
    default:
      if (verbose) {
        printf("Undefined test case.\n");
      }
      return true;
  }
  pointer = AddCmd(*memory, pointer, AsmAddi(A0, rd, 0));
  pointer = AddCmd(*memory, pointer, AsmXor(RA, RA, RA));
  pointer = AddCmd(*memory, pointer, AsmJalr(ZERO, RA, 0));

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
  std::map<R_TYPE_TEST, const std::string> test_name = {{TEST_ADD,   "ADD"},
                                                        {TEST_SUB,   "SUB"},
                                                        {TEST_AND,   "AND"},
                                                        {TEST_OR,    "OR"},
                                                        {TEST_XOR,   "XOR"},
                                                        {TEST_SLL,   "SLL"},
                                                        {TEST_SRL,   "SRL"},
                                                        {TEST_SRA,   "SRA"},
                                                        {TEST_SLT,   "SLT"},
                                                        {TEST_SLTU,  "SLTU"},
                                                        {TEST_ADDW,  "ADDW"},
                                                        {TEST_SLLW,  "SLLW"},
                                                        {TEST_SRAW,  "SRAW"},
                                                        {TEST_SRLW,  "SRLW"},
                                                        {TEST_SUBW,  "SUBW"},
                                                        {TEST_CADD,  "C.ADD"},
                                                        {TEST_CAND,  "C.AND"},
                                                        {TEST_CADDW, "C.ADDW"},
                                                        {TEST_COR,   "C.OR"},
                                                        {TEST_CSUB,  "C.SUB"},
                                                        {TEST_CSUBW, "C.SUBW"},
                                                        {TEST_CXOR,  "C.XOR"},
                                                        {TEST_CMV,   "C.MV"},

  };
  printf("%s test %s.\n", test_name[test_case].c_str(),
         error ? "failed" : "passed");
}

bool TestRTypeLoop(bool verbose = true) {
  bool total_error = false;
  R_TYPE_TEST test_sets[] = {TEST_ADD, TEST_SUB, TEST_AND, TEST_OR, TEST_XOR,
                             TEST_SLL, TEST_SRL, TEST_SRA,
                             TEST_SLT, TEST_SLTU, TEST_ADDW, TEST_SLLW,
                             TEST_SRAW, TEST_SRLW, TEST_SUBW,
                             TEST_CADD, TEST_CAND, TEST_CADDW,
                             TEST_COR, TEST_CSUB, TEST_CSUBW,
                             TEST_CXOR, TEST_CMV,
  };
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
  uint64_t pointer = 0;
  pointer = AddCmd(*memory, pointer, AsmJal(ZERO, offset));
  pointer += offset - 4;
  pointer = AddCmd(*memory, pointer, AsmAuipc(rd, val));
  pointer = AddCmd(*memory, pointer, AsmAddi(A0, rd, 0));
  pointer = AddCmd(*memory, pointer, AsmXor(RA, RA, RA));
  pointer = AddCmd(*memory, pointer, AsmJalr(ZERO, RA, 0));

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
  uint64_t pointer = 0;
  pointer = AddCmd(*memory, pointer, AsmAdd(A0, ZERO, 0));
  pointer = AddCmd(*memory, pointer, AsmLui(A0, val >> 12));
  pointer = AddCmd(*memory, pointer, AsmXor(RA, RA, RA));
  pointer = AddCmd(*memory, pointer, AsmJalr(ZERO, RA, 0));

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
  TEST_LB, TEST_LBU, TEST_LH, TEST_LHU, TEST_LW, TEST_LWU, TEST_LD,
  TEST_CLD, TEST_CLDSP, TEST_CLW, TEST_CLWSP,
};

bool TestLoad(LOAD_TEST test_type, uint32_t rd, uint32_t rs1, uint32_t offset0,
              uint32_t offset1, uint64_t val,
              bool verbose) {
  if (!en_64_bit && (test_type == TEST_LWU || test_type == TEST_LD)) {
    return false;
  }
  if (!en_ctest && (test_type == TEST_CLD || test_type == TEST_CLDSP
                    || test_type == TEST_CLWSP || test_type == TEST_CLW)) {
    return false;
  }
  bool error = false;
  std::string test_case = "";

  if (test_type == TEST_CLD) {
    rd = (rd & 0b111) + 8;
    rs1 = (rs1 & 0b111) + 8;
    offset1 = offset1 & 0b11111000;
  } else if (test_type == TEST_CLDSP) {
    rs1 = X2;
    offset1 = 0b0111111000;
  } else if (test_type == TEST_CLWSP) {
    rs1 = X2;
    offset1 = offset1 & 0b11111100;
    if (rd == 0) {
      return false;
    }
  } else if (test_type == TEST_CLW) {
    rs1 = (rs1 & 0b111) + 8;
    rd = (rd & 0b111) + 8;
    offset1 = offset1 & 0b1111100;
  }

  if (rs1 == ZERO) {
    offset0 = 0;
  }
  uint32_t address = offset0 + SignExtend(offset1, 12);
  for (int i = 0; i < 8; ++i) {
    memory->WriteByte(address + i, (val >> i * 8) & 0xFF);
  }
  // LW test code
  uint64_t pointer = 0;
  int32_t val20, val12;
  std::tie(val20, val12) = SplitImmediate(offset0);
  int64_t expected;
  pointer = AddCmd(*memory, pointer, AsmLui(rs1, val20));
  pointer = AddCmd(*memory, pointer, AsmAddi(rs1, rs1, val12));
  switch (test_type) {
    case TEST_LW:
      pointer = AddCmd(*memory, pointer, AsmLw(rd, rs1, offset1));
      expected = val & 0xFFFFFFFF;
      expected = SignExtend(expected, 32);
      break;
    case TEST_LWU:
      assert(xlen == 64);
      pointer = AddCmd(*memory, pointer, AsmLwu(rd, rs1, offset1));
      expected = val & 0xFFFFFFFF;
      break;
    case TEST_LB:
      pointer = AddCmd(*memory, pointer, AsmLb(rd, rs1, offset1));
      expected = val & 0xFF;
      expected = SignExtend(expected, 8);
      break;
    case TEST_LBU:
      pointer = AddCmd(*memory, pointer, AsmLbu(rd, rs1, offset1));
      expected = val & 0xFF;
      break;
    case TEST_LH:
      pointer = AddCmd(*memory, pointer, AsmLh(rd, rs1, offset1));
      expected = val & 0xFFFF;
      expected = SignExtend(expected, 16);
      break;
    case TEST_LHU:
      pointer = AddCmd(*memory, pointer, AsmLhu(rd, rs1, offset1));
      expected = val & 0xFFFF;
      break;
    case TEST_LD:
      pointer = AddCmd(*memory, pointer, AsmLd(rd, rs1, offset1));
      expected = val;
      break;
    case TEST_CLD:
      pointer = AddCmdCType(*memory, pointer, AsmCLd(rd, rs1, offset1));
      expected = val;
      break;
    case TEST_CLW:
      pointer = AddCmdCType(*memory, pointer, AsmCLw(rd, rs1, offset1));
      expected = val & 0xFFFFFFFF;
      expected = SignExtend(expected, 32);
      break;
    case TEST_CLDSP:
      pointer = AddCmdCType(*memory, pointer, AsmCLdsp(rd, offset1));
      expected = val;
      break;
    case TEST_CLWSP:
      pointer = AddCmdCType(*memory, pointer, AsmCLwsp(rd, offset1));
      expected = SignExtend(val & 0xFFFFFFFF, 32);
      break;
    default:
      if (verbose) {
        printf("Undefined test case %d\n", test_type);
        return true;
      }
  }
  pointer = AddCmd(*memory, pointer, AsmAddi(A0, rd, 0));
  pointer = AddCmd(*memory, pointer, AsmXor(RA, RA, RA));
  pointer = AddCmd(*memory, pointer, AsmJalr(ZERO, RA, 0));
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
      printf("rd: %2d, rs1: %2d, offset0: %08X, offset1: %08X, val: %08lX\n",
             rd,
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
  std::map<LOAD_TEST, const std::string> test_name = {{TEST_LB,    "LB"},
                                                      {TEST_LBU,   "LBU"},
                                                      {TEST_LH,    "LH"},
                                                      {TEST_LHU,   "LHU"},
                                                      {TEST_LW,    "LW"},
                                                      {TEST_LWU,   "LWU"},
                                                      {TEST_LD,    "LD"},
                                                      {TEST_CLD,   "CLD"},
                                                      {TEST_CLW,   "CLW"},
                                                      {TEST_CLDSP, "CLDSP"},
                                                      {TEST_CLWSP, "CLWSP"},
  };
  printf("%s test %s.\n", test_name[test_case].c_str(),
         error ? "failed" : "passed");
}

bool TestLoadLoop(bool verbose) {
  bool error = false;
  LOAD_TEST test_sets[] = {TEST_LB, TEST_LBU, TEST_LH, TEST_LHU, TEST_LW,
                           TEST_LWU, TEST_LD, TEST_CLD, TEST_CLW,
                           TEST_CLDSP, TEST_CLWSP};
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
        if (test_case == TEST_CLD) {
          offset1 &= 0b11111000;
        } else if (test_case == TEST_CLW) {
          offset1 &= 0b1111100;
        }
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
  TEST_SW,
  TEST_SH,
  TEST_SB,
  TEST_SD,
  TEST_CSD,
  TEST_CSDSP,
  TEST_CSW,
  TEST_CSWSP,
};

bool
TestStore(STORE_TEST test_type, uint32_t rs1, uint32_t rs2, uint32_t offset0,
          uint32_t offset1, uint32_t value,
          bool verbose) {
  if (!en_64_bit && (test_type == TEST_SD || test_type == TEST_CSD
                     || test_type == TEST_CSDSP)) {
    return false;
  }
  if (!en_ctest && (test_type == TEST_CSD || test_type == TEST_CSDSP
                    || test_type == TEST_CSW || test_type == TEST_CSWSP)) {
    return false;
  }
  bool error = false;
  std::string test_case = "";
  MemoryWrapper &mem = *memory;

  if (test_type == TEST_CSD) {
    rs1 = (rs1 & 0b111) + 8;
    rs2 = (rs2 & 0b111) + 8;
    assert((offset1 & ~0b11111000) == 0);
  } else if (test_type == TEST_CSDSP) {
    rs1 = X2;
    assert((offset1 & ~0b111111000) == 0);
  } else if (test_type == TEST_CSW) {
    rs1 = (rs1 & 0b111) + 8;
    rs2 = (rs1 & 0b111) + 8;
    assert((offset1 & ~0b1111100) == 0);
  } else if (test_type == TEST_CSWSP) {
    rs1 = X2;
    assert((offset1 & ~0b11111100) == 0);
  }

  if (rs1 == rs2) {
    value = offset0;
  }

  // STORE test code
  uint64_t pointer = 0;
  uint32_t val20, val12;
  std::tie(val20, val12) = SplitImmediate(value);
  pointer = AddCmd(*memory, pointer, AsmLui(rs2, val20));
  pointer = AddCmd(*memory, pointer, AsmAddi(rs2, rs2, val12));
  uint32_t offset20, offset12;
  std::tie(offset20, offset12) = SplitImmediate(offset0);
  pointer = AddCmd(*memory, pointer, AsmLui(rs1, offset20));
  pointer = AddCmd(*memory, pointer, AsmAddi(rs1, rs1, offset12));
  pointer = AddCmd(*memory, pointer, AsmSd(rs1, ZERO, offset1));
  int64_t expected;
  switch (test_type) {
    case TEST_SW:
      test_case = "SW";
      pointer = AddCmd(*memory, pointer, AsmSw(rs1, rs2, offset1));
      expected = value & 0xFFFFFFFF;
      break;
    case TEST_SH:
      test_case = "SH";
      pointer = AddCmd(*memory, pointer, AsmSh(rs1, rs2, offset1));
      expected = value & 0x0000FFFF;
      break;
    case TEST_SB:
      test_case = "SB";
      pointer = AddCmd(*memory, pointer, AsmSb(rs1, rs2, offset1));
      expected = value & 0x000000FF;
      break;
    case TEST_SD:
      test_case = "SD";
      pointer = AddCmd(*memory, pointer, AsmSd(rs1, rs2, offset1));
      // SD works with 64 bit. Extend to 64 bit boundary.
      expected = SignExtend(static_cast<uint64_t >(value), 32);
      break;
    case TEST_CSD:
      test_case = "C.SD";
      pointer = AddCmdCType(*memory, pointer, AsmCSd(rs1, rs2, offset1));
      expected = SignExtend(static_cast<uint64_t >(value), 32);
      break;
    case TEST_CSDSP:
      test_case = "C.CSDSP";
      pointer = AddCmdCType(*memory, pointer, AsmCSdsp(rs2, offset1));
      expected = SignExtend(static_cast<uint64_t >(value), 32);
      break;
    case TEST_CSW:
      test_case = "C.SW";
      pointer = AddCmdCType(*memory, pointer, AsmCsw(rs1, rs2, offset1));
      expected = value & 0xFFFFFFFF;
      break;
    case TEST_CSWSP:
      test_case = "C.SWSP";
      pointer = AddCmdCType(*memory, pointer, AsmCSwsp(rs2, offset1));
      expected = value & 0xFFFFFFFF;
      break;
    default:
      if (verbose) {
        printf("Undefined test case %d\n", test_type);
      }
      return true;
  }
  pointer = AddCmd(*memory, pointer, AsmAddi(A0, ZERO, 0));
  pointer = AddCmd(*memory, pointer, AsmXor(RA, RA, RA));
  pointer = AddCmd(*memory, pointer, AsmJalr(ZERO, RA, 0));
  expected = (rs2 == ZERO) ? 0 : expected;

  RiscvCpu cpu(en_64_bit);
  RandomizeRegisters(cpu);
  cpu.SetMemory(memory);
  error = cpu.RunCpu(0, verbose) != 0;
  offset0 = (rs1 == ZERO) ? 0 : offset0;
  uint32_t address = offset0 + SignExtend(offset1, 12);
  int size;
  switch (test_type) {
    case TEST_SB:
      size = 1;
      break;
    case TEST_SH:
      size = 2;
      break;
    case TEST_SW:
    case TEST_CSW:
    case TEST_CSWSP:
      size = 4;
      break;
    case TEST_SD:
    case TEST_CSD:
    case TEST_CSDSP:
      size = 8;
      break;
  }
  uint64_t result = 0;
  for (int i = 0; i < size; ++i) {
    result |= static_cast<uint64_t>(mem.ReadByte(address + i)) << (8 * i);
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
  std::map<STORE_TEST, const std::string> test_name = {{TEST_SW,    "SW"},
                                                       {TEST_SH,    "SH"},
                                                       {TEST_SB,    "SB"},
                                                       {TEST_SD,    "SD"},
                                                       {TEST_CSD,   "CSD"},
                                                       {TEST_CSDSP, "CSDSP"},
                                                       {TEST_CSW,   "CSW"},
                                                       {TEST_CSWSP, "CSWSP"},
  };
  printf("%s test %s.\n", test_name[test_case].c_str(),
         error ? "failed" : "passed");
}

bool TestStoreLoop(bool verbose) {
  bool error = false;
  STORE_TEST test_sets[] = {TEST_SW, TEST_SH, TEST_SB, TEST_SD, TEST_CSD,
                            TEST_CSDSP, TEST_CSW, TEST_CSWSP};
  for (auto test_case: test_sets) {
    for (int i = 0; i < kUnitTestMax && !error; i++) {
      int32_t rs1 = rnd() % 32;
      int32_t rs2 = rnd() % 32;
      uint32_t offset0 = 0, offset1 = 0, offset = 0;
      while (offset < 40 || offset >= kMemSize - 4) {
        int32_t offset0_effective;
        offset0 = rnd() % kMemSize;
        offset1 = rnd() & 0x0FFF;
        if (test_case == TEST_CSD) {
          offset1 &= 0b11111000;
        } else if (test_case == TEST_CSDSP) {
          offset1 &= 0b111111000;
        } else if (test_case == TEST_CSW) {
          offset1 &= 0b1111100;
        } else if (test_case == TEST_CSWSP) {
          offset1 &= 0b11111100;
        }
        offset0_effective = (rs1 == ZERO) ? 0 : offset0;
        offset = offset0_effective + SignExtend(offset1, 12);
      }
      int32_t value = rnd() & 0xFFFFFFFF;
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

// B-Type tests start here.
enum B_TYPE_TEST {
  TEST_BEQ, TEST_BGE, TEST_BGEU, TEST_BLT, TEST_BLTU, TEST_BNE, TEST_CBEQZ,
  TEST_CBNEZ, TEST_CJ, TEST_CJAL,
};
// c.j and c.jal are included here.

std::map<B_TYPE_TEST, const std::string> test_name = {{TEST_BEQ,   "BEQ"},
                                                      {TEST_BGE,   "BGE"},
                                                      {TEST_BGEU,  "BGEU"},
                                                      {TEST_BLT,   "BLT"},
                                                      {TEST_BLTU,  "BLTU"},
                                                      {TEST_BNE,   "BNE"},
                                                      {TEST_CBEQZ, "C.BEQZ"},
                                                      {TEST_CBNEZ, "C.BNEZ"},
                                                      {TEST_CJ,    "C.J"},
                                                      {TEST_CJAL,  "CJAL"},
};

void PrintBTypeInstructionMessage(B_TYPE_TEST test_case, bool error) {
  printf("%s test %s.\n", test_name[test_case].c_str(),
         error ? "failed" : "passed");
}

bool
TestBType(B_TYPE_TEST test_type, uint32_t rs1, uint32_t rs2, uint32_t value1,
          uint32_t value2, int32_t offset,
          bool verbose = true) {
  bool error = false;
  if (test_type == TEST_CJAL && en_64_bit) {
    return false;
  }
  if (!en_ctest && (test_type == TEST_CBNEZ || test_type == TEST_CBEQZ ||
                    test_type == TEST_CJ || test_type == TEST_CJAL)) {
    return false;
  }
  std::string test_case = test_name[test_type];

  if (test_type == TEST_CBEQZ || test_type == TEST_CBNEZ) {
    rs1 = (rs1 & 0b111) + 8;
    rs2 = 0;
  }
  if (test_type == TEST_CJ || test_type == TEST_CJAL) {
    rs1 = rs2 = 0;
  }

  value1 = (rs1 == ZERO) ? 0 : value1;
  value2 = (rs2 == ZERO) ? 0 : value2;
  value1 = (rs1 == rs2) ? value2 : value1;

  uint32_t start_point = kMemSize / 2;
  uint64_t pointer = start_point;
  uint32_t value20, value12;
  uint32_t expected;
  std::tie(value20, value12) = SplitImmediate(value1);
  pointer = AddCmd(*memory, pointer, AsmLui(rs1, value20));
  pointer = AddCmd(*memory, pointer, AsmAddi(rs1, rs1, value12));
  std::tie(value20, value12) = SplitImmediate(value2);
  pointer = AddCmd(*memory, pointer, AsmLui(rs2, value20));
  pointer = AddCmd(*memory, pointer, AsmAddi(rs2, rs2, value12));
  uint64_t next_pos = pointer + offset;
  if (test_type == TEST_BEQ) {
    pointer = AddCmd(*memory, pointer, AsmBeq(rs1, rs2, offset));
    expected = (value1 == value2) ? 1 : 0;
  } else if (test_type == TEST_BGE) {
    pointer = AddCmd(*memory, pointer, AsmBge(rs1, rs2, offset));
    expected = (static_cast<int32_t>(value1) >= static_cast<int32_t>(value2))
               ? 1 : 0;
  } else if (test_type == TEST_BGEU) {
    pointer = AddCmd(*memory, pointer, AsmBgeu(rs1, rs2, offset));
    expected = value1 >= value2 ? 1 : 0;
  } else if (test_type == TEST_BLT) {
    pointer = AddCmd(*memory, pointer, AsmBlt(rs1, rs2, offset));
    expected =
      static_cast<int32_t>(value1) < static_cast<int32_t>(value2) ? 1 : 0;
  } else if (test_type == TEST_BLTU) {
    pointer = AddCmd(*memory, pointer, AsmBltu(rs1, rs2, offset));
    expected = value1 < value2 ? 1 : 0;
  } else if (test_type == TEST_BNE) {
    pointer = AddCmd(*memory, pointer, AsmBne(rs1, rs2, offset));
    expected = value1 != value2 ? 1 : 0;
  } else if (test_type == TEST_CBEQZ) {
    pointer = AddCmdCType(*memory, pointer, AsmCBeqz(rs1, offset));
    expected = value1 == value2 ? 1 : 0;
  } else if (test_type == TEST_CBNEZ) {
    pointer = AddCmdCType(*memory, pointer, AsmCBnez(rs1, offset));
    expected = value1 != value2 ? 1 : 0;
  } else if (test_type == TEST_CJ) {
    pointer = AddCmdCType(*memory, pointer, AsmCJ(offset));
    expected = 1;
  } else if (test_type == TEST_CJAL) {
    pointer = AddCmdCType(*memory, pointer, AsmCJal(offset));
    expected = 1;
  }
  pointer = AddCmd(*memory, pointer, AsmAddi(A0, ZERO, 0));
  pointer = AddCmd(*memory, pointer, AsmXor(RA, RA, RA));
  pointer = AddCmd(*memory, pointer, AsmJalr(ZERO, RA, 0));
  pointer = next_pos;
  pointer = AddCmd(*memory, pointer, AsmAddi(A0, ZERO, 1));
  pointer = AddCmd(*memory, pointer, AsmXor(RA, RA, RA));
  pointer = AddCmd(*memory, pointer, AsmJalr(ZERO, RA, 0));


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
                             TEST_BNE, TEST_CBEQZ, TEST_CBNEZ, TEST_CJ,
                             TEST_CJAL};

  for (B_TYPE_TEST test_case: test_sets) {
    bool error = false;
    for (int i = 0; i < kUnitTestMax && !error; i++) {
      uint32_t rs1 = rnd() % 32;
      uint32_t rs2 = rnd() % 32;
      uint32_t value1;
      uint32_t value2;
      uint32_t equal = rnd() & 1;
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
        case TEST_CBEQZ:
        case TEST_CBNEZ:
          value1 = equal ? 0 : rnd();
          value2 = 0;
          break;
        case TEST_CJ:
        case TEST_CJAL:
          value1 = value2 = 0;
          break;
      }
      int32_t offset = 0;
      while (-64 < offset && offset < 64) {
        constexpr int kRange = 1 << 12;
        offset = 2 * ((rnd() % kRange) - kRange / 2);
        if (test_case == TEST_CBEQZ || test_case == TEST_CBNEZ) {
          offset = SignExtend(offset, 9);
        } else if (test_case == TEST_CJ || test_case == TEST_CJAL) {
          offset = SignExtend(offset, 12);
        }
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
enum JAL_TYPE_TEST {
  TEST_JAL, TEST_JALR, TEST_CJALR, TEST_CJR,
};

void PrintJalrTypeInstructionMessage(JAL_TYPE_TEST test_case, bool error,
                                     bool verbose = true) {
  if (!verbose) {
    return;
  }
  std::map<JAL_TYPE_TEST, const std::string> test_name = {{TEST_JAL,   "JAL"},
                                                          {TEST_JALR,  "JALR"},
                                                          {TEST_CJALR, "C.JALR"},
                                                          {TEST_CJR,   "C.JR"},
  };
  printf("%s test %s.\n", test_name[test_case].c_str(),
         error ? "failed" : "passed");
}

bool TestJalrType(JAL_TYPE_TEST test_type, uint32_t rd, uint32_t rs1,
                  uint32_t offset, uint32_t value,
                  bool verbose) {
  bool error = false;
  std::string test_case = "";
  if (!en_ctest && (test_type == TEST_CJR || test_type == TEST_CJALR)) {
    return false;
  }

  if (rs1 == ZERO) {
    value = 0;
  }
  if (test_type == TEST_CJR) {
    offset = 0;
    rd = ZERO;
  }
  if (test_type == TEST_CJALR) {
    offset = 0;
    rd = X1;
    if (rs1 == 0) {
      return false;
    }
  }

  uint32_t start_point = kMemSize / 4;
  uint64_t pointer = start_point;
  uint64_t rd_address;
  uint32_t value20, value12;
  std::tie(value20, value12) = SplitImmediate(value);
  pointer = AddCmd(*memory, pointer, AsmLui(rs1, value20));
  pointer = AddCmd(*memory, pointer, AsmAddi(rs1, rs1, value12));
  switch (test_type) {
    case TEST_JALR:
      pointer = AddCmd(*memory, pointer, AsmJalr(rd, rs1, offset));
      break;
    case TEST_CJALR:
      pointer = AddCmdCType(*memory, pointer, AsmCJalr(rs1));
      break;
    case TEST_CJR:
      pointer = AddCmdCType(*memory, pointer, AsmCJr(rs1));
      break;
  }
  rd_address = pointer;
  // Code should not reach here.
  pointer = AddCmd(*memory, pointer, AsmAddi(A0, ZERO, 1));
  pointer = AddCmd(*memory, pointer, AsmXor(RA, RA, RA));
  pointer = AddCmd(*memory, pointer, AsmJalr(ZERO, RA, 0));
  pointer = (value + SignExtend(offset, 12)) & ~1;
  pointer = AddCmd(*memory, pointer, AsmAddi(A0, ZERO, 2));
  pointer = AddCmd(*memory, pointer, AsmXor(RA, RA, RA));
  pointer = AddCmd(*memory, pointer, AsmJalr(ZERO, RA, 0));
  uint32_t expected = 2;
  RiscvCpu cpu(en_64_bit);
  RandomizeRegisters(cpu);
  cpu.SetMemory(memory);
  error = cpu.RunCpu(start_point, verbose) != 0;
  int64_t return_value = static_cast<int64_t>(cpu.ReadRegister(A0));
  error |= return_value != expected;
  if (rd != 0 && rd != RA && rd != A0) {
    uint32_t expect = rd_address;
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
  JAL_TYPE_TEST test_sets[] = {TEST_JALR, TEST_CJALR, TEST_CJR};
  for (auto test_case: test_sets) {
    for (int i = 0; i < kUnitTestMax && !error; i++) {
      uint32_t rs1 = rnd() % 32;
      uint32_t rd = rnd() % 32;
      uint32_t offset = rnd() % 0x1000;
      // if rs1 == 0, the offset is from address=0. Only positive values are valid.
      if (rs1 == 0) {
        offset &= 0x7FF;
      }
      uint32_t value = kMemSize / 2 + (rnd() % (kMemSize / 4));
      bool test_error = TestJalrType(test_case, rd, rs1, offset, value, false);
      if (test_error) {
        test_error = TestJalrType(test_case, rd, rs1, offset, value, true);
      }
      error |= test_error;
    }
    PrintJalrTypeInstructionMessage(test_case, error, verbose);
  }
  return error;
}

// Multiple-Type test cases start here.
enum MULT_TYPE_TEST {
  TEST_MUL, TEST_MULH, TEST_MULHSU, TEST_MULHU, TEST_MULW,
  TEST_DIV, TEST_DIVU, TEST_DIVUW, TEST_DIVW,
  TEST_REM, TEST_REMU, TEST_REMUW, TEST_REMW,
};


bool
TestMultType(MULT_TYPE_TEST test_type, int32_t rd, int32_t rs1, int32_t rs2,
             int64_t value1, int64_t value2,
             bool verbose) {
  bool w_instruction =
    test_type == TEST_MULW || test_type == TEST_DIVUW || test_type == TEST_DIVW
    || test_type == TEST_REMUW || test_type == TEST_REMW;

  if (!en_64_bit && w_instruction) {
    return false;
  }

  bool error = false;
  int64_t expected;
  std::string test_case = "";

  uint64_t pointer = 0;
  uint32_t val20, val12;
  std::tie(val20, val12) = SplitImmediate(value1);
  pointer = AddCmd(*memory, pointer, AsmLui(rs1, val20));
  pointer = AddCmd(*memory, pointer, AsmAddi(rs1, rs1, val12));
  std::tie(val20, val12) = SplitImmediate(value2);
  pointer = AddCmd(*memory, pointer, AsmLui(rs2, val20));
  pointer = AddCmd(*memory, pointer, AsmAddi(rs2, rs2, val12));

  if (rs1 == 0) {
    value1 = 0;
  }
  if (rs2 == 0) {
    value2 = 0;
  }
  if (rs1 == rs2) {
    value1 = value2;
  }
  __int128 temp128;
  switch (test_type) {
    case TEST_MUL:
      pointer = AddCmd(*memory, pointer, AsmMul(rd, rs1, rs2));
      expected = value1 * value2;
      test_case = "MUL";
      break;
    case TEST_MULH:
      pointer = AddCmd(*memory, pointer, AsmMulh(rd, rs1, rs2));
      if (xlen == 32) {
        expected = (value1 * value2) >> xlen;
      } else {
        temp128 = static_cast<__int128 >(value1) * value2;
        expected = temp128 >> xlen;
      }
      test_case = "MULH";
      break;
    case TEST_MULHSU:
      pointer = AddCmd(*memory, pointer, AsmMulhsu(rd, rs1, rs2));
      if (xlen == 32) {
        expected = (value1 * static_cast<uint64_t >(value2 & 0xFFFFFFFF))
          >> xlen;
      } else {
        temp128 =
          static_cast<__int128 >(value1) * static_cast<uint64_t>(value2);
        expected = temp128 >> xlen;
      }
      test_case = "MULHSU";
      break;
    case TEST_MULHU:
      pointer = AddCmd(*memory, pointer, AsmMulhu(rd, rs1, rs2));
      if (xlen == 32) {
        expected = static_cast<uint64_t>(value1 & 0xFFFFFFFF) *
                   static_cast<uint64_t >(value2 & 0xFFFFFFFF);
        expected = expected >> xlen;
      } else {
        temp128 = static_cast<__int128 >(static_cast<uint64_t >(value1)) *
                  static_cast<uint64_t>(value2);
        expected = temp128 >> xlen;
      }
      test_case = "MULHU";
      break;
    case TEST_MULW:
      pointer = AddCmd(*memory, pointer, AsmMulw(rd, rs1, rs2));
      expected = (value1 * value2) & 0xFFFFFFFF;
      if (expected >> 31) {
        expected |= 0xFFFFFFFF00000000;
      }
      test_case = "MULW";
      break;
    case TEST_DIV:
      pointer = AddCmd(*memory, pointer, AsmDiv(rd, rs1, rs2));
      if (value2 == 0) {
        expected = -1;
      } else {
        expected = value1 / value2;
      }
      test_case = "DIV";
      break;
    case TEST_DIVU:
      pointer = AddCmd(*memory, pointer, AsmDivu(rd, rs1, rs2));
      if (value2 == 0) {
        expected = ~0;
      } else if (xlen == 32) {
        expected =
          static_cast<uint64_t>(value1 & 0xFFFFFFFF) /
          static_cast<uint64_t>(value2 & 0xFFFFFFFF);
      } else {
        expected =
          static_cast<uint64_t>(value1) / static_cast<uint64_t>(value2);
      }
      test_case = "DIVU";
      break;
    case TEST_DIVUW:
      pointer = AddCmd(*memory, pointer, AsmDivuw(rd, rs1, rs2));
      if ((value2 & 0xFFFFFFFF) == 0) {
        expected = ~0;
      } else {
        expected =
          static_cast<uint64_t>(value1 & 0xFFFFFFFF) /
          static_cast<uint64_t>(value2 & 0xFFFFFFFF);
      }
      test_case = "DIVUW";
      break;
    case TEST_DIVW:
      pointer = AddCmd(*memory, pointer, AsmDivw(rd, rs1, rs2));
      if ((value2 & 0xFFFFFFFF) == 0) {
        expected = -1;
      } else {
        expected =
          static_cast<int64_t>(SignExtend(value1, 32)) /
          static_cast<int64_t>(SignExtend(value2, 32));
      }
      test_case = "DIVW";
      break;
    case TEST_REM:
      pointer = AddCmd(*memory, pointer, AsmRem(rd, rs1, rs2));
      if (value2 == 0) {
        expected = value1;
      } else {
        expected = value1 % value2;
      }
      test_case = "REM";
      break;
    case TEST_REMU:
      pointer = AddCmd(*memory, pointer, AsmRemu(rd, rs1, rs2));
      if (value2 == 0) {
        expected = value1;
      } else if (xlen == 32) {
        expected =
          static_cast<uint64_t>(value1 & 0xFFFFFFFF) %
          static_cast<uint64_t>(value2 & 0xFFFFFFFF);
      } else {
        expected =
          static_cast<uint64_t>(value1) % static_cast<uint64_t>(value2);
      }
      test_case = "REMU";
      break;
    case TEST_REMUW:
      pointer = AddCmd(*memory, pointer, AsmRemuw(rd, rs1, rs2));
      if ((value2 & 0xFFFFFFFF) == 0) {
        expected = value1;
      } else {
        expected =
          static_cast<uint64_t>(value1 & 0xFFFFFFFF) %
          static_cast<uint64_t>(value2 & 0xFFFFFFFF);
      }
      test_case = "REMUW";
      break;
    case TEST_REMW:
      pointer = AddCmd(*memory, pointer, AsmRemw(rd, rs1, rs2));
      if ((value2 & 0xFFFFFFFF) == 0) {
        expected = value1;
      } else {
        expected =
          static_cast<int64_t>(SignExtend(value1, 32)) %
          static_cast<int64_t>(SignExtend(value2, 32));
      }
      test_case = "REMW";
      break;
    default:
      if (verbose) {
        printf("Undefined test case.\n");
      }
      return true;
  }
  pointer = AddCmd(*memory, pointer, AsmAddi(A0, rd, 0));
  pointer = AddCmd(*memory, pointer, AsmXor(RA, RA, RA));
  pointer = AddCmd(*memory, pointer, AsmJalr(ZERO, RA, 0));

  if (rd == 0) {
    expected = 0;
  }
  if (xlen == 32 || w_instruction) {
    expected = SignExtend(expected, 32);
  }
  RiscvCpu cpu(en_64_bit);
  RandomizeRegisters(cpu);
  cpu.SetMemory(memory);
  error = cpu.RunCpu(0, verbose) != 0;
  int64_t return_value = static_cast<int64_t>(cpu.ReadRegister(A0));
  error |= return_value != expected;
  if (error & verbose) {
    printf("RD: %d, RS1: %d, RS2: %d, Value1: %ld(%08lx), value2: %ld(%08lx)\n",
           rd,
           rs1, rs2, value1, value1,
           value2, value2);
  }
  if (verbose) {
    PrintErrorMessage(test_case, error, expected, return_value);
  }
  return error;
}

void PrintMultTypeInstructionMessage(MULT_TYPE_TEST test_case, bool error) {
  std::map<MULT_TYPE_TEST, const std::string> test_name = {{TEST_MUL,    "MUL"},
                                                           {TEST_MULH,   "MULH"},
                                                           {TEST_MULHSU, "MULHSU"},
                                                           {TEST_MULHU,  "MULHU"},
                                                           {TEST_MULW,   "MULW"},
                                                           {TEST_DIV,    "DIV"},
                                                           {TEST_DIVU,   "DIVU"},
                                                           {TEST_DIVUW,  "DIVUW"},
                                                           {TEST_DIVW,   "DIVW"},
                                                           {TEST_REM,    "REM"},
                                                           {TEST_REMU,   "REMU"},
                                                           {TEST_REMUW,  "REMUW"},
                                                           {TEST_REMW,   "REMW"},
  };
  printf("%s test %s.\n", test_name[test_case].c_str(),
         error ? "failed" : "passed");
}

bool TestMultTypeLoop(bool verbose = true) {
  bool total_error = false;
  MULT_TYPE_TEST test_sets[] = {TEST_MUL, TEST_MULH, TEST_MULHSU, TEST_MULHU,
                                TEST_MULW,
                                TEST_DIV, TEST_DIVU, TEST_DIVUW, TEST_DIVW,
                                TEST_REM, TEST_REMU, TEST_REMUW, TEST_REMW,
  };
  for (MULT_TYPE_TEST test_case: test_sets) {
    bool error = false;
    for (int i = 0; i < kUnitTestMax && !error; i++) {
      int32_t rd = rnd() & 0x1F;
      int32_t rs1 = rnd() & 0x1F;
      int32_t rs2 = rnd() & 0x1F;
      int32_t value1 = static_cast<int32_t>(rnd());
      int32_t value2 = static_cast<int32_t>(rnd());
      bool test_error = TestMultType(test_case, rd, rs1, rs2, value1, value2,
                                     false);
      if (test_error && verbose) {
        test_error = TestMultType(test_case, rd, rs1, rs2, value1, value2,
                                  true);
      }
      error |= test_error;
    }
    if (verbose) {
      PrintMultTypeInstructionMessage(test_case, error);
    }
    total_error |= error;
  }
  return total_error;
}
// Multiple test cases end here.

// AMO test cases start here.
enum AMO_TEST {
  TEST_AMOADDD,
};

bool
TestAmoType(AMO_TEST test_type, uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t offset,
          int32_t value0, int32_t value1, uint32_t aq, uint32_t rl,
          bool verbose) {
  bool is_64_instruction = test_type == TEST_AMOADDD;
  if (!en_64_bit && is_64_instruction) {
    return false;
  }
  bool error = false;
  std::string test_case = "";
  MemoryWrapper &mem = *memory;

  offset = rs1 == 0 ? 0 : offset;
  if (is_64_instruction) {
    offset &= 0xFFFFFFF8;
    mem.Write64(offset, static_cast<uint64_t>(value0));
  } else {
    mem.Write32(offset, value0);
  }
  if (rs1 == 0) {
    offset = 0;
  }
  if (rs2 == rs1) {
    value1 = offset;
  } else if (rs2 == 0) {
    value1 = 0;
  }

  // Amo test code
  constexpr uint64_t kStartPoint = 0x1000;
  uint64_t pointer = kStartPoint;
  uint32_t val20, val12;
  std::tie(val20, val12) = SplitImmediate(value1);
  pointer = AddCmd(*memory, pointer, AsmLui(rs2, val20));
  pointer = AddCmd(*memory, pointer, AsmAddi(rs2, rs2, val12));
  uint32_t offset20, offset12;
  std::tie(offset20, offset12) = SplitImmediate(offset);
  pointer = AddCmd(*memory, pointer, AsmLui(rs1, offset20));
  pointer = AddCmd(*memory, pointer, AsmAddi(rs1, rs1, offset12));
  // expected0 == M[offset], expected1 == x[rd].
  int64_t expected0, expected1;
  switch (test_type) {
    case TEST_AMOADDD:
      test_case = "AMOADD.D";
      pointer = AddCmd(*memory, pointer, AsmAmoAddd(rd, rs1, rs2, aq, rl));
      expected0 = static_cast<uint64_t>(value0) + static_cast<uint64_t>(value1);
      expected1 = value0;
      break;
    default:
      if (verbose) {
        printf("Undefined test case %d\n", test_type);
      }
      return true;
  }
  pointer = AddCmd(*memory, pointer, AsmAddi(A0, rd, 0));
  pointer = AddCmd(*memory, pointer, AsmXor(RA, RA, RA));
  pointer = AddCmd(*memory, pointer, AsmJalr(ZERO, RA, 0));
  expected1 = rd == ZERO ? 0 : expected1;

  RiscvCpu cpu(en_64_bit);
  RandomizeRegisters(cpu);
  cpu.SetMemory(memory);
  error = cpu.RunCpu(kStartPoint, verbose) != 0;
  uint64_t result = 0;
  if (is_64_instruction) {
    result = mem.Read64(offset);
  } else {
    result = mem.Read32(offset);
  }
  error |= result != expected0;
  error |= cpu.ReadRegister(A0) != expected1;
  if (verbose) {
    PrintErrorMessage(test_case, error, expected0, result);
    if (error) {
      printf("rd: %2d, rs1: %2d, rs2: %2d, offset: %08X, value0: %08X, value1: %08X\n",
             rd, rs1, rs2, offset, value0, value1);
    }
  }
  return error;
}

void PrintAmoInstructionMessage(AMO_TEST test_case, bool error,
                                  bool verbose = true) {
  if (!verbose) {
    return;
  }
  std::map<AMO_TEST, const std::string> test_name = {{TEST_AMOADDD, "AMOADD.D"},
  };
  printf("%s test %s.\n", test_name[test_case].c_str(),
         error ? "failed" : "passed");
}

bool TestAmoTypeLoop(bool verbose) {
  bool error = false;
  AMO_TEST test_sets[] = {TEST_AMOADDD, };
  for (auto test_case: test_sets) {
    for (int i = 0; i < kUnitTestMax && !error; i++) {
      int32_t rd = rnd() % 32;
      int32_t rs1 = rnd() % 32;
      int32_t rs2 = rnd() % 32;
      uint32_t aq = rnd() & 0b1;
      uint32_t rl = rnd() & 0b1;
      constexpr uint32_t kProgramStart = 0x1000;
      constexpr uint32_t kProgramEnd = 0x1FFF;
      uint32_t offset;
      do {
        offset = rnd() & 0xFFFFFFFC;
      } while (kProgramStart <= offset && offset < kProgramEnd);
      int32_t value0 = rnd() & 0xFFFFFFFF;
      int32_t value1 = rnd() & 0xFFFFFFFF;
      bool test_error = TestAmoType(test_case, rd, rs1, rs2, offset, value0, value1, aq, rl,
                                  false);
      if (test_error && verbose) {
        test_error = TestAmoType(test_case, rd, rs1, rs2, offset, value0, value1, aq, rl,
                                    true);
      }
      error |= test_error;
    }
    PrintAmoInstructionMessage(test_case, error, verbose);
  }
  return error;
}
// Amo type test cases end here.

// Summation test starts here.
bool TestSum(bool verbose) {
  uint64_t pointer = 0;
  LoadAssemblerSum(*memory, pointer);
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
  uint64_t assembly_pointer = 0;
  LoadAssemblerSort(*memory, assembly_pointer);

  constexpr int kArraySize = 100;
  constexpr int kArrayAddress = 512;
  uint64_t value_pointer = kArrayAddress;
  for (int i = 0; i < kArraySize * 4; i++) {
    int value = static_cast<int>(rnd() % 1000);
    memory->Write32(value_pointer + 4 * i, value);
  }

  if (verbose) {
    std::cout << "Before:\n";
    for (int i = 0; i < kArraySize; i++) {
      std::cout << memory->Read32(value_pointer + i * 4) << "\t";
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
    error_flag |= memory->Read32(value_pointer + i * 4) >
                  memory->Read32(value_pointer + i * 4 + 4);
  }

  if (verbose) {
    printf("After:\n");
    for (int i = 0; i < kArraySize; i++) {
      int32_t data = memory->Read32(value_pointer + i * 4);
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
      std::cout << "------- 64bit test start -------" << std::endl;
    } else {
      xlen = 32;
      std::cout << "------- 32bit test start -------" << std::endl;
    }
    error |= TestITypeLoop(verbose);
    error |= TestRTypeLoop(verbose);
    error |= TestLuiLoop(verbose);
    error |= TestAuipcLoop(verbose);
    error |= TestLoadLoop(verbose);
    error |= TestStoreLoop(verbose);
    error |= TestBTypeLoop(verbose);
    error |= TestJalrTypeLoop(verbose);
    error |= TestMultTypeLoop(verbose);
    error |= TestAmoTypeLoop(verbose);
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

} // namespace anonymous

int main() {
  return RunTest();
}
