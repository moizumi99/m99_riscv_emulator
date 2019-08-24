#ifndef RISCV_Emulator_H
#define RISCV_Emulator_H

enum CmdLabel {
  MOV = 0,
  ADD = 1,
  SUB = 2,
  AND = 3,
  OR = 4,
  SL = 5,
  SR = 6,
  SRA = 7,
  LDL = 8,
  LDH = 9,
  CMP = 10,
  JE = 11,
  JMP = 12,
  LD = 13,
  ST = 14,
  HLT = 15
};

#endif // RISCV_Emulator_H