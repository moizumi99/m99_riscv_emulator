#include "RISCV_Emulator.h"
#include "load_assembler.h"
#include "RISCV_cpu.h"
#include <iostream>

using namespace std;

// Code
uint32_t mem[256];

int main() {
  // Generate very primitive assembly code
  load_assembler_sum(mem);
  printf("Assembler set.\n");

  // Run CPU emulator
  printf("Execution start\n");
  int return_value = run_cpu(mem);

  printf("Return value: %d\n", return_value);

  return return_value;
}