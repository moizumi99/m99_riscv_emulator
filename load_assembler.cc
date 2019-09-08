#include "load_assembler.h"
#include "RISCV_Emulator.h"
#include "RISCV_cpu.h"
#include "bit_tools.h"
#include "instruction_encdec.h"
#include "assembler.h"

using namespace std;

void load_assembler_sum(uint32_t *mem) {
  mem[0] = asm_addi(T0, ZERO, 0);
  mem[1] = asm_addi(T1, ZERO, 0);
  mem[2] = asm_addi(T2, ZERO, 10);
  mem[3] = asm_addi(T0, T0, 1);
  mem[4] = asm_add(T1, T1, T0);
  mem[5] = asm_beq(T0, T2, 8);
  mem[6] = asm_jal(ZERO, -16);
  mem[7] = asm_add(A0, T1, ZERO);
  mem[8] = asm_jalr(ZERO, RA, 0);
}
