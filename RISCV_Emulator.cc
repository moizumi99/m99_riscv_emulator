#include "RISCV_Emulator.h"
#include "load_assembler.h"
#include "RISCV_cpu.h"
#include <iostream>

using namespace std;

// Code
uint8_t mem[256];

int main() {
  // Generate very primitive assembly code
  load_assembler_sum(mem);
  printf("Assembler set.\n");

  // Run CPU emulator
  printf("Execution start\n");
  int error = run_cpu(mem, 0);
  if (error) {
    printf("CPU execution fail.\n");
  }
  int return_value = read_register(A0);

  printf("Return value: %d\n", return_value);

  return return_value;
}