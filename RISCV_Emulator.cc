#include "RISCV_Emulator.h"
#include "load_assembler.h"
#include "RISCV_cpu.h"
#include <iostream>

using namespace std;

// Code
uint32_t mem[256];

int main() {
  // Generate very primitive assembly code
  load_assembler(mem);

  // Run CPU emulator
  int error_code = run_cpu(mem);

  return error_code;
}