#include "RISCV_cpu.h"
#include "assembler.h"
#include "bit_tools.h"

MemorWrapperIterator AddCmd(MemorWrapperIterator &mem, uint32_t cmd) {

  *(mem++) = cmd & 0xFF;
  *(mem++) = (cmd >> 8) & 0xFF;
  *(mem++) = (cmd >> 16) & 0xFF;
  *(mem++) = (cmd >> 24) & 0xFF;
  return mem;
}

MemorWrapperIterator LoadAssemblerSum(MemorWrapperIterator &mem) {
  AddCmd(mem, AsmAddi(T0, ZERO, 0));
  AddCmd(mem, AsmAddi(T1, ZERO, 0));
  AddCmd(mem, AsmAddi(T2, ZERO, 10));
  AddCmd(mem, AsmAddi(T0, T0, 1));
  AddCmd(mem, AsmAdd(T1, T1, T0));
  AddCmd(mem, AsmBeq(T0, T2, 8));
  AddCmd(mem, AsmJal(ZERO, -16));
  AddCmd(mem, AsmAdd(A0, T1, ZERO));
  AddCmd(mem, AsmXor(RA, RA, RA));
  AddCmd(mem, AsmJalr(ZERO, RA, 0));

  return mem;
}

MemorWrapperIterator LoadAssemblerSort(MemorWrapperIterator &mem) {
  // A1 is n and A3 points to A[0]
  // A4 is i, A5 is j, a6 is x
  AddCmd(mem, AsmAddi(A3, A0, 4)); // 0
  AddCmd(mem, AsmAddi(A4, X0, 1)); // 4
  // Outer Loop
  AddCmd(mem, AsmBltu(A4, A1, +8)); // 8
  // Exit Outer Loop
  AddCmd(mem, AsmJalr(X0, X1, 0)); // 0c
  // Continue Outer Loop
  AddCmd(mem, AsmLw(A6, A3, 0));   // 10
  AddCmd(mem, AsmAddi(A2, A3, 0)); // 14
  AddCmd(mem, AsmAddi(A5, A4, 0)); // 18
  // Inner Loop
  AddCmd(mem, AsmLw(A7, A2, -4));  // 1c
  AddCmd(mem, AsmBge(A6, A7, +20));  // 20
  // sw rs2, offset(rs1) <= Note the rs1 / rs2 order.
  AddCmd(mem, AsmSw(A2, A7, 0));  // 24
  AddCmd(mem, AsmAddi(A5, A5, -1)); // 28
  AddCmd(mem, AsmAddi(A2, A2, -4));  // 2c
  AddCmd(mem, AsmBne(A5, X0, -20));  // 30
  // Exit Inner Loop
  AddCmd(mem, AsmSlli(A5, A5, 2));  // 34
  AddCmd(mem, AsmAdd(A5, A0, A5));  // 38
  AddCmd(mem, AsmSw(A5, A6, 0));  // 3c
  AddCmd(mem, AsmAddi(A4, A4, 1));  // 40
  AddCmd(mem, AsmAddi(A3, A3, 4));  // 44
  AddCmd(mem, AsmJal(X0, -64));  // 48
  return mem;
}
