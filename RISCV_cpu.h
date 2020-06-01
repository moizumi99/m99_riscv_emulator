#ifndef RISCV_CPU_H
#define RISCV_CPU_H

#include <cstdint>
#include <utility>
#include <vector>
#include <memory>
#include "riscv_cpu_common.h"
#include "bit_tools.h"
#include "memory_wrapper.h"

namespace RISCV_EMULATOR {

class RiscvCpu {
  static constexpr int kCsrSize = 4096;
  static constexpr int kRegSize = 32;
  static constexpr int kRegNum = 32;
  int xlen_;
  int mxl_ = 1;
public:
  RiscvCpu(bool en64bit);

  RiscvCpu();

  ~RiscvCpu() {

  };

  void SetRegister(uint32_t num, uint64_t value);

  uint64_t ReadRegister(uint32_t num);

  void SetMemory(std::shared_ptr<MemoryWrapper> memory);

  void SetCsr(uint32_t index, uint64_t value);

  uint64_t ReadCsr(uint32_t index);

  int RunCpu(uint64_t start_pc, bool verbose = true);

  static std::tuple<uint32_t, uint32_t, uint32_t, uint32_t, int32_t> GetCode16(uint32_t ir, int mxl);

private:
  uint64_t
  VirtualToPhysical(uint64_t virtual_address, bool write_access = false);

  uint64_t Sext32bit(uint64_t data32bit);

  uint64_t reg_[kRegSize];
  uint64_t pc_;
  uint64_t next_pc_;
  uint64_t mstatus_;
  PrivilegeMode privilege_;
  std::shared_ptr<MemoryWrapper> memory_;
  std::vector<uint64_t> csrs_;
  bool ctype_;

  void InitializeCsrs();

  void Ecall();

  void CsrsInstruction(uint32_t instruction, uint32_t csr, uint32_t rd,
                       uint32_t rs1);

  uint64_t BranchInstruction(uint32_t instruction, uint32_t rs1, uint32_t rs2,
                             int32_t imm13);

  void OperationInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1,
                            uint32_t rs2);

  void ImmediateInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1,
                            int32_t imm12);

  void
  ImmediateShiftInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1,
                            uint32_t shamt);

  void LoadInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1,
                       int32_t imm12);

  void StoreInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1,
                        uint32_t rs2, int32_t imm12_stype);

  void SystemInstruction(uint32_t instruction, uint32_t rd, int32_t imm12);

  void MultInstruction(uint32_t instruction, uint32_t rd, uint32_t rs1,
                            uint32_t rs2);

  void Mret();

  void Sret();

  uint32_t LoadCmd(uint64_t pc);

  uint32_t GetCode32(uint32_t ir);


  std::pair<bool, bool> SystemCall();

  uint64_t LoadWd(uint64_t physical_address, int width = 32);

  void StoreWd(uint64_t physical_address, uint64_t data, int width = 32);

  void Trap(int cause = 0, bool interrupt = false);

  bool page_fault_ = false;
  bool prev_page_fault_ = false;
  uint64_t prev_faulting_address_ = 0;
  bool error_flag_, end_flag_;
  uint64_t faulting_address_;

  inline bool CheckShiftSign(uint8_t shamt, uint8_t instruction,
                             const std::string &message_str);

  PrivilegeMode IntToPrivilegeMode(int value);

  void DumpPrivilegeStatus();

  void DumpCpuStatus();

  void DumpRegisters();

  void UpdateMstatus(int16_t csr);

  void ApplyMstatusToCsr();
  // Below are for system call and host emulation
public:
  void SetWorkMemory(uint64_t top, uint64_t bottom);

  void SetEcallEmulationEnable(
    bool ecall_emulation) { ecall_emulation_ = ecall_emulation; };

  void SetHostEmulationEnable(
    bool host_emulation) { host_emulation_ = host_emulation; };
private:
  void HostEmulation();

  static constexpr uint64_t kToHost = 0x80001000;
  static constexpr uint64_t kFromHost = 0x80001040;
  bool ecall_emulation_ = false;
  bool host_emulation_ = false;
  bool host_write_;
  uint64_t top_ = 0x80000000;
  uint64_t bottom_ = 0x40000000;
  uint64_t brk_ = bottom_;
};

enum ExceptionCode {
  INSTRUCTION_ADDRESS_MISALIGNED = 0,
  INSTRUCTION_ACCESS_FAULT = 1,
  ILLEGAL_INSTRUCTION = 2,
  BREAK_POINT = 3,
  LOAD_ADDRESS_MISALIGNED = 4,
  LOAD_ACCESS_FAULT = 5,
  STORE_ADDRESS_MISALIGNED = 6,
  STORE_ACCESS_FAULT = 7,
  ECALL_UMODE = 8,
  ECALL_SMODE = 9,
  ECALL_MMODE = 11,
  INSTRUCTION_PAGE_FAULT = 12,
  LOAD_PAGE_FAULT = 13,
  STORE_PAGE_FAULT = 15
};

enum CsrsAddresses {
  // User Trap Handling
  USTATUS = 0x000, // User status register.
  UIE = 0x004, // User interrupt-enable register.
  UTVEC = 0x005, // User trap handler base address.
  USCRATCH = 0x040, // Scratch register for user trap handlers.
  UEPC = 0x041, // User exception program counter.
  UCAUSE = 0x042, // User trap cause.
  UTVAL = 0x43, // User bad address or instruction.
  UIP = 0x44, // User interrupt pending.
  // Supervisor Trap Handling.
  SSTATUS = 0x100, // Supervisor status register.
  SEDELEG = 0x102, // Supervisor exception delegation register.
  SIDELEG = 0X103, // Supervisor interrupt delegation register.
  SIE = 0x104, // Supervisor interrupt-enable register.
  STVEC = 0x105, // Supervisor trap handler base address.
  SCOUNTEREN = 0x106, // Supervisor counter enable.
  SSCRATCH = 0x140, // Scratch register for supervisor trap handlers.
  SEPC = 0x141, // Supervisor exception program counter.
  SCAUSE = 0x142, // Supervisor trap cause,
  STVAL = 0x143, // Supervisor bad address or instruction.
  SIP = 0x144, // Supervisor interrupt pending.
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
  FUNC_MULT = 0b0000001,
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
  FUNC3_MUL = 0b000,
  FUNC3_MULH = 0b001,
  FUNC3_MULHSU = 0b010,
  FUNC3_MULHU = 0b011,
  FUNC3_DIV = 0b100,
  FUNC3_DIVU = 0b101,
  FUNC3_REM = 0b110,
  FUNC3_REMU = 0b111,
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
  INST_FENCEI,
  // RV32M/RV64M instructions
  INST_MUL,
  INST_MULH,
  INST_MULHSU,
  INST_MULHU,
  INST_MULW,
  INST_DIV,
  INST_DIVU,
  INST_DIVUW,
  INST_DIVW,
  INST_REM,
  INST_REMU,
  INST_REMUW,
  INST_REMW,
};

} // namespace RISCV_EMULATOR

#endif // RISCV_CPU_H