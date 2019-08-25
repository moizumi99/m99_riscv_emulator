#include "RISCV_Emulator.h"
#include "assembler.h"
#include "RISCV_cpu.h"
#include <iostream>

using namespace std;

uint32_t rom[256];

int main() {
  assembler(rom);
  int error_code = run_cpu(rom);
  return error_code;
}