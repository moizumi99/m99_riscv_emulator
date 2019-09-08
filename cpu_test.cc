
#include "RISCV_cpu.h"
#include "load_assembler.h"
#include <stdint.h>
#include <iostream>

using namespace std;

namespace cpu_test {

uint32_t mem[0x010000];

bool test_sum(bool verbose) {
  load_assembler_sum(mem);
  constexpr int kExpectedValue = 55;
  int return_value = run_cpu(mem, verbose);
  bool error = return_value != kExpectedValue;
  if (error) {
    printf("Sum test failed\n");
  }
  return error;
}

} // namespace cpu_test

int main() {
  bool verbose = false;

  bool error = false;
  error |= cpu_test::test_sum(verbose);

  if (!error) {
    printf("All CPU Tests passed.\n");
  }
  return error;
}