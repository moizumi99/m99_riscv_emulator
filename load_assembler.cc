#include "RISCV_cpu.h"
#include "assembler.h"
#include "bit_tools.h"

using namespace RISCV_EMULATOR;

namespace CPU_TEST {

uint64_t AddCmd(MemoryWrapper &mem, uint64_t address, uint32_t cmd) {
  mem.WriteByte(address++, cmd & 0xFF);
  mem.WriteByte(address++, (cmd >> 8) & 0xFF);
  mem.WriteByte(address++, (cmd >> 16) & 0xFF);
  mem.WriteByte(address++, (cmd >> 24) & 0xFF);
  return address;
}

uint64_t AddCmdCType(MemoryWrapper &mem, uint64_t address, uint16_t cmd) {
  mem.WriteByte(address++, cmd & 0xFF);
  mem.WriteByte(address++, (cmd >> 8) & 0xFF);
  return address;
}

uint64_t LoadAssemblerSum(MemoryWrapper &mem, uint64_t address) {
  address = AddCmd(mem, address, AsmAddi(T0, ZERO, 0));
  address = AddCmd(mem, address, AsmAddi(T1, ZERO, 0));
  address = AddCmd(mem, address, AsmAddi(T2, ZERO, 10));
  address = AddCmd(mem, address, AsmAddi(T0, T0, 1));
  address = AddCmd(mem, address, AsmAdd(T1, T1, T0));
  address = AddCmd(mem, address, AsmBeq(T0, T2, 8));
  address = AddCmd(mem, address, AsmJal(ZERO, -16));
  address = AddCmd(mem, address, AsmAdd(A0, T1, ZERO));
  address = AddCmd(mem, address, AsmXor(RA, RA, RA));
  address = AddCmd(mem, address, AsmJalr(ZERO, RA, 0));

  return address;
}

uint64_t LoadAssemblerSort(MemoryWrapper &mem, uint64_t address) {
  // A1 is n and A3 points to A[0]
  // A4 is i, A5 is j, a6 is x
  address = AddCmd(mem, address, AsmAddi(A3, A0, 4)); // 0
  address = AddCmd(mem, address, AsmAddi(A4, X0, 1)); // 4
  // Outer Loop
  address = AddCmd(mem, address, AsmBltu(A4, A1, +8)); // 8
  // Exit Outer Loop
  address = AddCmd(mem, address, AsmJalr(X0, X1, 0)); // 0c
  // Continue Outer Loop
  address = AddCmd(mem, address, AsmLw(A6, A3, 0));   // 10
  address = AddCmd(mem, address, AsmAddi(A2, A3, 0)); // 14
  address = AddCmd(mem, address, AsmAddi(A5, A4, 0)); // 18
  // Inner Loop
  address = AddCmd(mem, address, AsmLw(A7, A2, -4));  // 1c
  address = AddCmd(mem, address, AsmBge(A6, A7, +20));  // 20
  // sw rs2, offset(rs1) <= Note the rs1 / rs2 order.
  address = AddCmd(mem, address, AsmSw(A2, A7, 0));  // 24
  address = AddCmd(mem, address, AsmAddi(A5, A5, -1)); // 28
  address = AddCmd(mem, address, AsmAddi(A2, A2, -4));  // 2c
  address = AddCmd(mem, address, AsmBne(A5, X0, -20));  // 30
  // Exit Inner Loop
  address = AddCmd(mem, address, AsmSlli(A5, A5, 2));  // 34
  address = AddCmd(mem, address, AsmAdd(A5, A0, A5));  // 38
  address = AddCmd(mem, address, AsmSw(A5, A6, 0));  // 3c
  address = AddCmd(mem, address, AsmAddi(A4, A4, 1));  // 40
  address = AddCmd(mem, address, AsmAddi(A3, A3, 4));  // 44
  address = AddCmd(mem, address, AsmJal(X0, -64));  // 48
  return address;
}

} // namespace RISCV_EMULATOR