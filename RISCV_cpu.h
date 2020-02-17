#ifndef RISCV_CPU_H
#define RISCV_CPU_H

#include <cstdint>
#include <utility>
#include <vector>
#include <memory>
#include "bit_tools.h"
#include "memory_wrapper.h"

class RiscvCpu {
  static constexpr int kCsrSize = 4096;
  static constexpr int kRegSize = 32;
  static constexpr int kRegNum = 32;
public:
  RiscvCpu();
  ~RiscvCpu() {};

  void set_register(uint32_t num, uint32_t value);
  uint32_t read_register(uint32_t num);
  void set_memory(std::shared_ptr<memory_wrapper> memory);
  void set_csr(uint32_t index, uint32_t value);
  uint32_t read_csr(uint32_t index);
  int run_cpu(uint32_t start_pc, bool verbose = true);
  uint32_t VirtualToPhysical(uint32_t virtual_address, bool write_access = false);
protected:
  inline bool check_shift_sign(bool x, const std::string &message_str);

private:
  uint32_t reg[kRegSize];
  uint32_t pc;
  std::shared_ptr<memory_wrapper> memory;
  std::vector<uint32_t> csrs;
  uint32_t load_cmd(uint32_t pc);
  uint32_t get_code(uint32_t ir);
  std::pair<bool, bool> system_call();
  uint32_t load_wd(uint32_t virtual_address);
  void store_wd(uint32_t virtual_address, uint32_t data, int width = 32);

  // Below are for system call emulation
public:
  void set_work_memory(uint32_t top, uint32_t bottom);
private:
  uint32_t top = 0x80000000;
  uint32_t bottom = 0x40000000;
  uint32_t brk = bottom;
};

enum CsrsAddresses {
  // Super visor Protection and Translation.
  kSptbr = 0x180, // Page-table base register. Former satp register.
  // Machine Trap Handling.
  kMscratch = 0x340, // Scratch register for machine trap handlers.
  kMepc = 0x341, // Machine exception program counter.
  // TDOD: add other CSR addresses.
  // https://people.eecs.berkeley.edu/~krste/papers/riscv-privileged-v1.9.1.pdf
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
  OPCODE_ARITHLOG_I = 0b00010011,
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
  INST_AND,
  INST_SUB,
  INST_OR,
  INST_XOR,
  INST_SLL,
  INST_SRL,
  INST_SRA,
  INST_SLT,
  INST_SLTU,
  INST_ADDI,
  INST_ANDI,
  INST_ORI,
  INST_XORI,
  INST_SLLI,
  INST_SRLI,
  INST_SRAI,
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
  INST_SW,
  INST_SH,
  INST_SB,
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