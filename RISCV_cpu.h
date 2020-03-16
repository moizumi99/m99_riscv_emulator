#ifndef RISCV_CPU_H
#define RISCV_CPU_H

#include <cstdint>
#include <utility>
#include <vector>
#include <memory>
#include "bit_tools.h"
#include "memory_wrapper.h"

constexpr int kXlen = 64;

enum PrivilegeModes {
  USER_LEVEL = 0,
  SUPERVISOR_LEVEL = 1,
  MACHINE_LEVEL = 3
};

class RiscvCpu {
  static constexpr int kCsrSize = 4096;
  static constexpr int kRegSize = 32;
  static constexpr int kRegNum = 32;
public:
  RiscvCpu();
  ~RiscvCpu() {};

  void SetRegister(uint32_t num, uint64_t value);
  uint64_t ReadRegister(uint32_t num);
  void SetMemory(std::shared_ptr<MemoryWrapper> memory);
  void SetCsr(uint32_t index, uint64_t value);
  uint64_t ReadCsr(uint32_t index);
  int RunCpu(uint64_t start_pc, bool verbose = true);
  uint64_t VirtualToPhysical(uint64_t virtual_address, bool write_access = false);

private:
  uint64_t reg_[kRegSize];
  uint64_t pc_;
  uint8_t privilege_ = MACHINE_LEVEL;
  std::shared_ptr<MemoryWrapper> memory_;
  std::vector<uint64_t> csrs_;
  uint32_t LoadCmd(uint32_t pc);
  uint32_t GetCode(uint32_t ir);
  std::pair<bool, bool> SystemCall();
  uint64_t LoadWd(uint64_t virtual_address, int width = 32);
  void StoreWd(uint64_t virtual_address, uint64_t data, int width = 32);
  void InstructionPageFault();
  bool page_fault_ = false;
  bool prev_page_fault_ = false;
  bool error_flag_, end_flag_;
  inline bool CheckShiftSign(uint8_t shamt, uint8_t instruction, const std::string &message_str);

  // Below are for system call emulation
public:
  void SetWorkMemory(uint32_t top, uint32_t bottom);
private:
  uint64_t top_ = 0x80000000;
  uint64_t bottom_ = 0x40000000;
  uint64_t brk_ = bottom_;
};

enum CsrsAddresses {
  // Supervisor Trap Handling
  SEPC = 0x141, // Supervisor exception program counter.
  // Super visor Protection and Translation.
  SATP = 0x180, // Page-table base register. Former satp register.
  // Machine Trap Setup
  MSTATUS = 0x300, // Machine status register.
  MISA = 0x301, // ISA and extensions.
  MEDELEG = 0x302, // Machine exception delegation register.
  MIDELEG = 0x303, // Machine interrupt delegation register.
  MIE = 0x304, // Machine interrupt-enable register.
  MTVEC = 0x305, // Machine trap-handler base address.
  MCOUNTEREN = 0x306, // Machine counter enable.
  // Machine Trap Handling.
  MSCRATCH = 0x340, // Scratch register for machine trap handlers.
  MEPC = 0x341, // Machine exception program counter.
  MCAUSE = 0x342, // Machine trap cause.
  MTVAL = 0x343, // Machine bad address
  MIP = 0x344, // Machine interrupt pending
  // TDOD: add other CSR addresses.
  // https://riscv.org/specifications/privileged-isa/
};

enum Registers {
  ZERO = 0,
  X0 = 0,
  X1 = 1,
  X2 = 2,
  X3 = 3,
  X4 = 4,
  X5 = 5,
  X6 = 6,
  X7 = 7,
  X8 = 8,
  X9 = 9,
  X10 = 10,
  X11 = 11,
  X12 = 12,
  X13 = 13,
  X14 = 14,
  X15 = 15,
  X16 = 16,
  RA = 1,
  SP = 2,
  GP = 3,
  TP = 4,
  T0 = 5,
  T1 = 6,
  T2 = 7,
  FP = 8,
  S0 = 8,
  S1 = 9,
  A0 = 10,
  A1 = 11,
  A2 = 12,
  A3 = 13,
  A4 = 14,
  A5 = 15,
  A6 = 16,
  A7 = 17,
  S2 = 18,
  S3 = 19,
  S4 = 20,
  S5 = 21,
  S6 = 22,
  S7 = 23,
  S8 = 24,
  S9 = 25,
  S10 = 26,
  S11 = 27,
  T3 = 28,
  T4 = 29,
  T5 = 30,
  T6 = 31
};

enum op_label {
  OPCODE_ARITHLOG = 0b00110011,
  OPCODE_ARITHLOG_64 = 0b00111011,
  OPCODE_ARITHLOG_I = 0b00010011,
  OPCODE_ARITHLOG_I64 = 0b0011011,
  OPCODE_B = 0b01100011,
  OPCODE_LD = 0b00000011,
  OPCODE_J = 0b01101111,
  OPCODE_S = 0b00100011,
  OPCODE_JALR = 0b01100111,
  OPCODE_LUI = 0b00110111,
  OPCODE_AUIPC = 0b00010111,
  OPCODE_SYSTEM = 0b01110011,
  OPCODE_FENCE = 0b0001111,
};

enum op_funct {
  FUNC_NORM = 0b0000000,
  FUNC_ALT = 0b0100000,
  FUNC_MRET = 0b0011000,
};

enum op_funct3 {
  FUNC3_ADDSUB = 0b000,
  FUNC3_AND = 0b111,
  FUNC3_OR = 0b110,
  FUNC3_XOR = 0b100,
  FUNC3_SL = 0b001,
  FUNC3_SR = 0b0101,
  FUNC3_SLT = 0b010,
  FUNC3_SLTU = 0b011,
  FUNC3_BEQ = 0b000,
  FUNC3_BGE = 0b101,
  FUNC3_BGEU = 0b111,
  FUNC3_BLT = 0b100,
  FUNC3_BLTU = 0b110,
  FUNC3_BNE = 0b001,
  FUNC3_LSB = 0b000,
  FUNC3_LSBU = 0b100,
  FUNC3_LSH = 0b001,
  FUNC3_LSHU = 0b101,
  FUNC3_LSW = 0b010,
  FUNC3_LSD = 0b011,
  FUNC3_LSWU = 0b110,
  FUNC3_JALR = 0b000,
  FUNC3_SYSTEM = 0b000,
  FUNC3_CSRRC = 0b011,
  FUNC3_CSRRCI = 0b111,
  FUNC3_CSRRS = 0b010,
  FUNC3_CSRRSI = 0b110,
  FUNC3_CSRRW = 0b001,
  FUNC3_CSRRWI = 0b101,
  FUNC3_FENCE = 0b000,
  FUNC3_FENCEI = 0b001,
};

enum instruction {
  INST_ERROR,
  INST_ADD,
  INST_ADDW,
  INST_AND,
  INST_SUB,
  INST_SUBW,
  INST_OR,
  INST_XOR,
  INST_SLL,
  INST_SLLW,
  INST_SRL,
  INST_SRLW,
  INST_SRA,
  INST_SRAW,
  INST_SLT,
  INST_SLTU,
  INST_ADDI,
  INST_ADDIW,
  INST_ANDI,
  INST_ORI,
  INST_XORI,
  INST_SLLI,
  INST_SLLIW,
  INST_SRLI,
  INST_SRLIW,
  INST_SRAI,
  INST_SRAIW,
  INST_SLTI,
  INST_SLTIU,
  INST_BEQ,
  INST_BGE,
  INST_BGEU,
  INST_BLT,
  INST_BLTU,
  INST_BNE,
  INST_JAL,
  INST_JALR,
  INST_LB,
  INST_LBU,
  INST_LH,
  INST_LHU,
  INST_LW,
  INST_LWU,
  INST_LD,
  INST_SB,
  INST_SH,
  INST_SW,
  INST_SD,
  INST_LUI,
  INST_AUIPC,
  INST_SYSTEM,
  INST_CSRRC,
  INST_CSRRCI,
  INST_CSRRS,
  INST_CSRRSI,
  INST_CSRRW,
  INST_CSRRWI,
  INST_FENCE,
  INST_FENCEI
};


#endif // RISCV_CPU_H